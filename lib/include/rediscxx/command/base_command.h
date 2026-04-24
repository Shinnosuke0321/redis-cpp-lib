//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <string>
#include <hiredis/async.h>
#include <functional>
#include "core/memory/intrusive_ptr.h"

namespace rediscxx {
    enum class command {
        set, get, hset, hget, hgetall, auth, select, incr,
        srem, sadd, smembers, exat, expire, expireat, flushall, flushdb, del,
        multi, exec, discard, watch
    };

    class command_executor_base : public core::ref_counted<command_executor_base> {
    public:
        virtual void execute(redisAsyncContext*) = 0;
        ~command_executor_base() override = default;
    };

    class base_command {
    public:
        virtual ~base_command() = default;
        virtual command get_cmd_type() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;
    };

#define COMMAND_CLASS_TYPE(cmd) \
    static std::string_view get_static_str() noexcept { return #cmd; } \
    static command get_static_type() noexcept { return command::cmd; } \
    command get_cmd_type() const noexcept override { return get_static_type(); }; \
    std::string to_string() const noexcept override { return std::string(get_static_str()); }
}
