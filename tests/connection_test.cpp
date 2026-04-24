//
// Created by Shinnosuke Kawai on 3/30/26.
//
#include "suites/connection_suite.h"
#include <rediscxx/client.h>
#include <rediscxx/event/event_loop_executer.h>
#include <latch>

class counting_executer final : public rediscxx::event::event_loop_executer {
public:
    std::expected<void, rediscxx::event::event_loop_error> init() noexcept override {
        ++init_count;
        return event_loop_executer::init();
    }
    int init_count = 0;
};

TEST_F(RedisCxxTest, SuccessfullWithValidConfig) {
    rediscxx::client::config config{"127.0.0.1", 6379, "redis-auth-password", 1};
    rediscxx::client client(std::move(config));
    std::expected<void, core::error::exception> result = client.connect();
    ASSERT_TRUE(result.has_value()) << result.error().to_what();
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongPassword) {
    rediscxx::client::config config{"127.0.0.1", 6379, "wrongpassword",1};
    rediscxx::client client(std::move(config));
    std::expected<void, core::error::exception> result = client.connect();
    ASSERT_FALSE(result.has_value());
    std::println("{}", result.error().to_what());
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongPort) {
    rediscxx::client::config config{"127.0.0.1", 0000, "redis-auth-password",1};
    rediscxx::client client(std::move(config));
    std::expected<void, core::error::exception> result = client.connect();
    ASSERT_FALSE(result.has_value());
    std::println("{}", result.error().to_what());
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, WrongHost) {
    rediscxx::client::config config{"123.1.2.3", 6379, "redis-auth-password",1};
    rediscxx::client client(std::move(config));
    std::expected<void, core::error::exception> result = client.connect();
    ASSERT_FALSE(result.has_value());
    std::println("{}", result.error().to_what());
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, InvalidDbIndex) {
    rediscxx::client::config config{"127.0.0.1", 6379, "redis-auth-password",123};
    rediscxx::client client(std::move(config));
    std::expected<void, core::error::exception> result = client.connect();
    ASSERT_FALSE(result.has_value());
    std::println("{}", result.error().to_what());
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, ConcurrentConnectNeverDeadlocks) {
    rediscxx::client::config config{"127.0.0.1", 6379, "redis-auth-password",1};
    rediscxx::client client(std::move(config));
    constexpr size_t num_t = 1000;
    std::atomic success_count{0};
    std::atomic failure_count{0};
    std::vector<std::thread> threads(num_t);
    for (auto& t : threads) {
        t = std::thread([&]() {
            if (const auto res = client.connect(); !res) {
                failure_count.fetch_add(1, std::memory_order_relaxed);
            } else {
                success_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    ASSERT_EQ(success_count.load(), num_t);
    ASSERT_EQ(failure_count.load(), 0);
    std::println("{} ms", timer.elapsed_milliseconds());
}

TEST_F(RedisCxxTest, ConcurrentConnectInitCalledOnce) {
    // counting_executer derives from ref_counted<event_loop_executer>, not ref_counted<counting_executer>,
    // so make_intrusive<counting_executer> won't satisfy the concept. Use raw new + intrusive_from_this().
    auto* spy = new counting_executer();
    rediscxx::client::config config{"127.0.0.1", 6379, "redis-auth-password", 1};
    rediscxx::client client(std::move(config), spy->intrusive_from_this());

    constexpr int N = 100;
    std::atomic success_count{0};
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; ++i) {
        threads.emplace_back([&] {
            if (client.connect())
                success_count.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads)
        t.join();

    EXPECT_EQ(spy->init_count, 1);
    EXPECT_EQ(success_count.load(), N);
    std::println("{} ms", timer.elapsed_milliseconds());
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
