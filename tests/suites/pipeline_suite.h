//
// Created by Shinnosuke Kawai on 4/4/26.
//

#pragma once

#include "redis_cxx_suite.h"

class PipelineTest: public RedisCxxTest {
protected:
    void SetUp() override {
        RedisCxxTest::SetUp();
        for (int i = 0; i < key_count; ++i) {
            keys[i] = std::format("pipeline_key{}", i);
        }
        for (int i = 0; i < key_count; ++i) {
            values[i] = std::format("pipeline_value{}", i);
        }
    }
    void TearDown() override {
        RedisCxxTest::TearDown();
        keys.clear();
        values.clear();
    }

    size_t key_count = 100;
    std::vector<std::string> keys{key_count};
    std::vector<std::string> values{key_count};
};