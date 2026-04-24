//
// Created by Shinnosuke Kawai on 4/2/26.
//
#pragma once
#include <gtest/gtest.h>
#include <rediscxx/client.h>
#include "redis_cxx_suite.h"

#define FLUSH_DB(client) \
    auto flush_future = client.execute_command(rediscxx::command::flushdb);\
    auto flush_res = flush_future.get();\
    ASSERT_FALSE(!flush_res) << flush_res.error().to_string();

class RedisCommandTest: public RedisCxxTest {
protected:
    void SetUp() override {
        RedisCxxTest::SetUp();
    }
    void TearDown() override {
        RedisCxxTest::TearDown();
    }
};