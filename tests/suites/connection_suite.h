//
// Created by Shinnosuke Kawai on 3/31/26.
//
#pragma once
#include "redis_cxx_suite.h"

class ConnectionTest: public RedisCxxTest {
protected:
    void SetUp() override {
        RedisCxxTest::SetUp();
    }
    void TearDown() override {
        RedisCxxTest::TearDown();
    }
};