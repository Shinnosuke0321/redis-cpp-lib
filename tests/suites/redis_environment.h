//
// Created by Shinnosuke Kawai on 3/30/26.
//
#pragma once
#include <gtest/gtest.h>
#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
inline int setenv(const char* name, const char* value, int overwrite)
{
    if (!overwrite) {
        size_t envsize = 0;
        errno_t err = getenv_s(&envsize, NULL, 0, name);
        if (err == 0 && envsize != 0) return 0; // Variable already exists, do not overwrite
    }
    std::string env_var = std::string(name) + "=" + std::string(value);
    return _putenv(env_var.c_str());
}
#else
#include <cstdlib> // for setenv on non-Windows systems
#endif

class RedisEnvironment : public testing::Environment {
public:
    void SetUp() override {
        setenv("REDIS_HOST", "127.0.0.1", 1);
        setenv("REDIS_PORT", "6379", 1);
        setenv("REDIS_PASSWORD", "redis", 1);
    }
};