//
// Created by Shinnosuke Kawai on 4/4/26.
//

#pragma once
#include "util/timer.h"
#include <gtest/gtest.h>
#include <rediscxx/client.h>

class RedisCxxTest: public testing::Test {
protected:
    void SetUp() override {
        timer.reset();
    }

    void TearDown() override {
    }

    Timer timer{};
    rediscxx::client::config m_config{"127.0.0.1", 6379, "redis-auth-password", 0};;
};