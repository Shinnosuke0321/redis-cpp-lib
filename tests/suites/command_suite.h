//
// Created by Shinnosuke Kawai on 4/2/26.
//
#pragma once
#include <gtest/gtest.h>
#include <rediscxx/client.h>
#include "redis_cxx_suite.h"

#define FLUSH_DB(client) \
    auto flush_future = client.execute_command(rediscxx::command::flush_db);\
    auto flush_res = flush_future.get();\
    ASSERT_TRUE(flush_res.has_value()) << flush_res.error().to_str();\
    ASSERT_EQ(flush_res.value().as_string().value(), "OK")

class RedisCommandTest: public RedisCxxTest {
protected:
    void SetUp() override {
        RedisCxxTest::SetUp();
    }
    void TearDown() override {

    }
};