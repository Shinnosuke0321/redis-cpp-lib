//
// Created by Shinnosuke Kawai on 3/31/26.
//
#pragma once
#include "redis_cxx_suite.h"

class ConnectionTest: public testing::Test {
protected:
    void SetUp() override {
        // RedisCxxTest::SetUp();
        // throw std::runtime_error("test");
    }
    void TearDown() override {
        // RedisCxxTest::TearDown();
    }
};