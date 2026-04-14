//
// Created by Shinnosuke Kawai on 4/2/26.
//
#include "suites/command_suite.h"

TEST_F(RedisCommandTest, SetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::set, "RedisCommand", "SetCommand");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_str();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_EQ(reply.as_string().value(), "OK");
    timer.print_elapsed_time();
}

TEST_F(RedisCommandTest, GetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::get, "RedisCommand");

    auto res = future.get();
    ASSERT_TRUE(res.has_value()) << res.error().to_str();
    rediscxx::result::reply& reply = res.value();
    ASSERT_TRUE(reply.as_string().has_value());
    ASSERT_EQ(reply.as_string().value(), "SetCommand");
    timer.print_elapsed_time();
}

TEST_F(RedisCommandTest, KeyWithNoValue) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::get, "KeyWithNoValue");

    auto res = future.get();
    ASSERT_TRUE(res.has_value()) << res.error().to_str();
    rediscxx::result::reply& reply = res.value();
    ASSERT_FALSE(reply.is_okay());
    ASSERT_FALSE(reply.as_string().has_value());
    timer.print_elapsed_time();
}

TEST_F(RedisCommandTest, HSetCommand) {
    CREATE_CLIENT(m_config);
    FLUSH_DB(client);
    auto future = client.execute_command(rediscxx::command::hset, "Hash", "key", "value", "key2", "value2");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_str();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_EQ(reply.as_integer().value(), 2);
}

TEST_F(RedisCommandTest, HGetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::hget, "Hash", "key");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_str();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_TRUE(reply.as_string().has_value());
    ASSERT_EQ(reply.as_string().value(), "value");
}

TEST_F(RedisCommandTest, HGetAll) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::hgetall, "Hash");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_str();
    rediscxx::result::reply& reply = expected.value();
    std::optional<rediscxx::result::reply::map> map = reply.as_map();
    ASSERT_TRUE(map.has_value());
    ASSERT_EQ(map.value().size(), 2u);
    for (auto& [key, value] : map.value()) {
        std::println("{}: {}", key, value);
    }
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
