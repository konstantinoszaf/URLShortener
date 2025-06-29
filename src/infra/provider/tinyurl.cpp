#include "infra/provider/tinyurl.h"
#include "infra/exception/exceptions.h"
#include <boost/json.hpp>
#include <iostream>

namespace json = boost::json;

TinyURL::TinyURL(std::shared_ptr<IHttpClient> client,
                std::shared_ptr<IEnvReader> env,
                std::shared_ptr<ICacheClient> redis_)
 : Provider(client, ProviderType::TINYURL, env, "url", redis_) {
    RequestInfo info {
        "api.tinyurl.com",
        "/create",
        "https",
        "Bearer " + api_key
    };

    request_info = info;
};

std::string TinyURL::get_short_url(std::string_view payload) {
    auto j = json::parse(payload);
    const json::object& obj = j.as_object();

    if (!obj.contains("data")) {
        throw URLShortener::exception::ProviderException(
            "Internal Server Error",
            HTTP::code::InternalServerError
        );
    }

    const json::object& data = obj.at("data").as_object();
    if (!data.contains("tiny_url")) {
        throw URLShortener::exception::ProviderException(
            "Internal Server Error",
            HTTP::code::InternalServerError
        );
    }

    return data.at("tiny_url").as_string().c_str();
}
