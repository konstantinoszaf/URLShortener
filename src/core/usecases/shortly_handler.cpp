#include "core/usecases/shortly_handler.h"
#include "core/ports/provider_interface.h"
#include "core/ports/provider_factory_interface.h"
#include "infra/exception/exceptions.h"
#include "infra/parser/json_parser.h"
#include "adapters/outgoing/boost_http_adapter.h"
#include <boost/json.hpp>
#include <iostream>
#include <boost/asio/awaitable.hpp>

namespace json = boost::json;

namespace {
void create_response(Response& response,
                    unsigned int result,
                    std::string_view reason,
                    HTTP::code code,
                    const json::object& payload) {
    json::object response_json;
    response_json["result"] = result;
    response_json["reason"] = reason;

    if (!payload.empty()) {
        response_json["data"] = payload;
    }

    response.status_code = code;
    response.headers["Content-Type"] = "application/json";
    response.body = json::serialize(response_json);
}
} // end of noname namespace

ShortlyHandler::ShortlyHandler(std::shared_ptr<IParser> parser,
                                std::shared_ptr<IProviderFactory> provider)
: json_parser(parser), provider_factory(provider) {}

async_task<void> ShortlyHandler::handle(const Request& request, Response& response) {
    try {
        if (request.body.empty()) {
            throw URLShortener::exception::ValidationException("Request body is empty");
        }

        auto [url, provider_type] = json_parser->parse(request.body);
        auto provider = provider_factory->createProvider(provider_type);

        std::string short_url = co_await provider->shorten(url);

        json::object data_object;
        data_object["url"] = url;
        data_object["shortened"] = short_url;

        create_response(response, 0, "", HTTP::code::OK, data_object);

    } catch (const URLShortener::exception::ValidationException& e) {
        std::cout << "Error: " << e.what() << '\n';
        create_response(response, 1, e.what(), HTTP::code::BadRequest, {});
    } catch (const URLShortener::exception::ProviderException& e) {
        std::cout << "What: " << e.what() << ", status: " << static_cast<unsigned int>(e.code()) << '\n';
        create_response(response, 1, e.what(), e.code(), {});
    } catch (const boost::system::error_code e) {
        std:: cout << "Boost error: " << e.message() << '\n';
        create_response(response, 1, e.message(), HTTP::code::InternalServerError, {});
    }
    co_return;
}
