#include <gtest/gtest.h>
#include "mock_http_client.h"
#include "mock_env_reader.h"
#include "infra/provider/bitly.h"
#include "infra/exception/exceptions.h"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <memory>

class TestableBitly : public Bitly {
public:
    using Bitly::Bitly;
    using Bitly::api_key;
    using Bitly::request_info;
    using Bitly::json_key;
};

class TestBitly : public ::testing::Test {
public:
    std::shared_ptr<MockHttpClient> http_client;
    std::shared_ptr<MockEnvReader> env;
    Response response;
    void SetUp() override {
        env = std::make_shared<MockEnvReader>();
        EXPECT_CALL(*env, get(::testing::_))
        .WillRepeatedly(::testing::Return("TOKEN"));
        http_client = std::make_shared<MockHttpClient>();
    }

    void TearDown() override {}
};

TEST_F(TestBitly, ReadValidToken) {
    std::string expected_token = "TOKEN";
    std::string expected_json_key = "long_url";

    TestableBitly provider{http_client, env};

    EXPECT_EQ(expected_token, provider.api_key);
    EXPECT_EQ(expected_json_key, provider.json_key);
}

TEST_F(TestBitly, ReadEmptyToken) {
    EXPECT_CALL(*env, get(::testing::_)).Times(1).WillOnce(::testing::Return(std::string{}));
    EXPECT_THROW((Bitly{http_client, env}), URLShortener::exception::ProviderException);
}

TEST_F(TestBitly, SendASuccessful200PostRequest) {
    std::string expected_url = "test.gr";
    std::string expected_body = R"({"long_url":")" + expected_url + R"("})";
    Response expected_response;
    expected_response.status_code = HTTP::code::OK;
    expected_response.body = R"({"link":"short.ly/abcd"})";;

    EXPECT_CALL(*http_client, post(::testing::Eq(expected_body), ::testing::_))
        .WillOnce(::testing::Invoke([expected_response](std::string_view, RequestInfo&) -> async_task<Response> {
            co_return expected_response;
        }));

    TestableBitly provider{http_client, env};

    boost::asio::io_context ioc;
    boost::asio::co_spawn(ioc, [&]() -> boost::asio::awaitable<void> {
        TestableBitly provider{http_client, env};
        std::string result = co_await provider.shorten(expected_url);
        EXPECT_EQ("short.ly/abcd", result);
        co_return;
    }, boost::asio::detached);

    ioc.run();
}

TEST_F(TestBitly, ShortenThrowsOnEmptyUrl) {
    TestableBitly provider{http_client, env};

    boost::asio::io_context ioc;
    bool exception_thrown = false;

    boost::asio::co_spawn(ioc, [&]() -> boost::asio::awaitable<void> {
        try {
            co_await provider.shorten("");
        } catch (const URLShortener::exception::ProviderException& e) {
            EXPECT_STREQ("url value can not be empty", e.what());
            EXPECT_EQ(e.code(), HTTP::code::BadRequest);
            exception_thrown = true;
        }
        co_return;
    }, boost::asio::detached);

    ioc.run();
    EXPECT_TRUE(exception_thrown);
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}