//
// Created by Shinnosuke Kawai on 4/2/26.
//
#include <latch>

#include "suites/command_suite.h"

#define CREATE_CLIENT(config) \
    rediscxx::client client(config); \
    auto result = client.connect(); \
    ASSERT_TRUE(result.has_value()) << result.error().to_what()


TEST_F(RedisCommandTest, SetCommandReturnsOk) {
    CREATE_CLIENT(m_config);
    auto set_exp = client.execute_command(rediscxx::command::set, "RedisCommand", "SetCommand").get();
    ASSERT_TRUE(set_exp) << set_exp.error().to_string();
    rediscxx::result::reply& reply = set_exp.value();
    ASSERT_TRUE(reply.is_okay());
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

TEST_F(RedisCommandTest, HGetNullKeyReturnsNull) {
    CREATE_CLIENT(m_config);
    auto hget_exp = client.execute_command(rediscxx::command::hget, "fake_hash", "null_key").get();
    ASSERT_TRUE(hget_exp) << hget_exp.error().to_string();
    rediscxx::result::reply& reply = hget_exp.value();
    ASSERT_TRUE(reply.is_nil());
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

TEST_F(RedisCommandTest, ExatCommand) {
    CREATE_CLIENT(m_config);
    auto exat_future = client.execute_command(rediscxx::command::expire, "Hash", "3");
    auto expected = exat_future.get();
    ASSERT_TRUE(expected.has_value()) << expected.error().to_string();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto hgetall_exp = client.execute_command(rediscxx::command::hgetall, "Hash").get();
    ASSERT_TRUE(hgetall_exp) << hgetall_exp.error().to_string();
    rediscxx::result::reply& reply = hgetall_exp.value();
    ASSERT_EQ(reply.as_map().value().size(), 0u);
}

TEST_F(RedisCommandTest, IncrementCounterConcurrently) {
    CREATE_CLIENT(m_config);
    auto set_exp = client.execute_command(rediscxx::command::set, "counter", "0").get();
    ASSERT_TRUE(set_exp.has_value()) << set_exp.error().to_string();
    ASSERT_TRUE(set_exp.value().is_okay());
    constexpr size_t num_t = 1000;
    std::latch latch(num_t);
    std::vector<std::thread> threads(num_t);
    for (auto& t : threads) {
        t = std::thread([&]() {
            latch.arrive_and_wait();
            auto incr_exp = client.execute_command(rediscxx::command::incr, "counter").get();
            ASSERT_TRUE(incr_exp.has_value()) << incr_exp.error().to_string();
            const rediscxx::result::reply& reply = incr_exp.value();
            ASSERT_TRUE(reply.as_integer().has_value());
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    auto get_exp = client.execute_command(rediscxx::command::get, "counter").get();
    ASSERT_TRUE(get_exp.has_value()) << get_exp.error().to_string();
    ASSERT_EQ(get_exp.value().as_string().value(), "1000");
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
