//
// Created by Shinnosuke Kawai on 4/5/26.
//
#pragma once
#include "redis_cxx_suite.h"

class TransactionTest: public RedisCxxTest {
protected:
    void SetUp() override {
        RedisCxxTest::SetUp();
    }
    void TearDown() override {
        RedisCxxTest::TearDown();
    }
};