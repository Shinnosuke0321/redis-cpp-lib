//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <string>
#include <hiredis/async.h>

namespace rediscxx {
    enum class command {
        set, get, hset, hgetall, sadd, smembers, auth, select,
        srem, exat, expireat, hget, flush_all, flush_db, del,
        multi, exec, discard, watch
    };

#define COMMAND_CLASS_TYPE(cmd) \
    std::string_view get_name() const noexcept override { return #cmd; } \
    command get_cmd_type() const noexcept override { return command::cmd; }; \
    std::string to_string() const noexcept override { return std::string(get_name());}

    class  command_base {
    public:
        virtual void execute(redisAsyncContext*) = 0;
        virtual ~command_base() = default;
        virtual std::string_view get_name() const noexcept = 0;
        virtual command get_cmd_type() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;
    };

    inline std::string_view command_to_str(const command cmd) noexcept {
        switch (cmd) {
            case command::set: return "set";
            case command::get: return "get";
            case command::hset: return "hset";
            case command::hgetall: return "hgetall";
            case command::sadd: return "sadd";
            case command::smembers: return "smembers";
            case command::auth: return "auth";
            case command::select: return "select";
            case command::srem: return "srem";
            case command::exat: return "exat";
            case command::expireat: return "expireat";
            case command::hget: return "hget";
            case command::flush_all: return "flush_all";
            case command::flush_db: return "flush_db";
            case command::del: return "del";
            case command::multi: return "multi";
            case command::exec: return "exec";
            case command::discard: return "discard";
            case command::watch: return "watch";
        }
        return "";
    }
}
