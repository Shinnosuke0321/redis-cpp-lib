//
// Created by Shinnosuke Kawai on 3/30/26.
//
#include "suites/connection_suite.h"
#include <rediscxx/client.h>

TEST_F(RedisCxxTest, SuccessfullWithValidConfig) {
    rediscxx::client::config config{"127.0.0.1", 6379, "redis-auth-password", 1};
    rediscxx::client client(std::move(config));
    std::expected<void, Core::Database::ConnectionError> result = client.connect();
    ASSERT_TRUE(result.has_value()) << result.error().to_str();
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongPassword) {
    rediscxx::client::config config{"127.0.0.1", 6379, "wrongpassword",1};
    rediscxx::client client(std::move(config));
    std::expected<void, Core::Database::ConnectionError> result = client.connect();
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().get_code(), Core::Database::ConnectionError::Type::AuthFailed);
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongPort) {
    rediscxx::client::config config{"127.0.0.1", 0000, "redis-auth-password",1};
    rediscxx::client client(std::move(config));
    std::expected<void, Core::Database::ConnectionError> result = client.connect();
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().get_code(), Core::Database::ConnectionError::Type::ConnectionFailed);
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongHost) {
    rediscxx::client::config config{"123.1.2.3", 6379, "redis-auth-password",1};
    rediscxx::client client(std::move(config));
    std::expected<void, Core::Database::ConnectionError> result = client.connect();
    ASSERT_FALSE(result.has_value());
    std::println("{} ms", timer.elapsed_milliseconds());
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
