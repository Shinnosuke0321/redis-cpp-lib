//
// Created by Shinnosuke Kawai on 4/2/26.
//
#include "suites/command_suite.h"

#define CREATE_CLIENT(config) \
    rediscxx::client client(config); \
    auto result = client.connect(); \
    ASSERT_TRUE(result.has_value()) << result.error().to_what()


TEST_F(RedisCommandTest, SetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::set, "RedisCommand", "SetCommand");
    auto expected = future.get();
    ASSERT_TRUE(expected) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_EQ(reply.as_string().value(), "OK");
    timer.print_elapsed_time();
}

TEST_F(RedisCommandTest, GetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::get, "RedisCommand");

    auto res = future.get();
    ASSERT_TRUE(res.has_value()) << res.error().to_string();
    rediscxx::result::reply& reply = res.value();
    ASSERT_TRUE(reply.as_string().has_value());
    ASSERT_EQ(reply.as_string().value(), "SetCommand");
    timer.print_elapsed_time();
}

TEST_F(RedisCommandTest, HSetCommand) {
    CREATE_CLIENT(m_config);
    FLUSH_DB(client);
    auto future = client.execute_command(rediscxx::command::hset, "Hash", "key", "value", "key2", "value2");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_EQ(reply.as_integer().value(), 2);
}

TEST_F(RedisCommandTest, HGetCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::hget, "Hash", "key");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    ASSERT_TRUE(reply.as_string().has_value());
    ASSERT_EQ(reply.as_string().value(), "value");
}

TEST_F(RedisCommandTest, HGetAllCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::hgetall, "Hash");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    std::optional<rediscxx::result::reply::map> map = reply.as_map();
    ASSERT_TRUE(map.has_value());
    ASSERT_EQ(map.value().size(), 2u);
    for (auto& [key, value] : map.value()) {
        std::println("{}: {}", key, value);
    }
}

TEST_F(RedisCommandTest, SAddCommand) {
    CREATE_CLIENT(m_config);
    std::vector<std::string> values = {"value1", "value2", "value3", "value4"};
    auto future = client.execute_command(rediscxx::command::sadd, "test_set", values);
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    std::optional<int64_t> map = reply.as_integer();
    ASSERT_TRUE(map.has_value());
    ASSERT_EQ(map.value(), 4);
}

TEST_F(RedisCommandTest, SMembersCommand) {
    CREATE_CLIENT(m_config);
    auto future = client.execute_command(rediscxx::command::smembers, "test_set");
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    std::optional<rediscxx::result::reply::set> set = reply.as_set();
    ASSERT_TRUE(set.has_value());
    ASSERT_EQ(set.value().size(), 4);
    for (auto& value : set.value()) {
        std::println("{}", value);
    }
}

TEST_F(RedisCommandTest, SRemCommand) {
    CREATE_CLIENT(m_config);
    std::vector<std::string> values = {"value1", "value2", "value3", "value4"};
    auto future = client.execute_command(rediscxx::command::srem, "test_set", values);
    auto expected = future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    rediscxx::result::reply& reply = expected.value();
    std::optional<int64_t> map = reply.as_integer();
    ASSERT_TRUE(map.has_value());
    ASSERT_EQ(map.value(), 4);
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
