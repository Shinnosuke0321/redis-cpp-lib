//
// Created by Shinnosuke Kawai on 12/3/25.
//

#pragma once
#include "redis_lib.h"

namespace RedisCmd {
    inline constexpr auto SET = "set";
    inline constexpr auto GET = "get";
    inline constexpr auto HSET = "hset";
    inline constexpr auto HGETALL = "hgetall";
    inline constexpr auto SADD = "sadd";
    inline constexpr auto SMEMBERS = "smembers";
    inline constexpr auto SREM = "srem";
    inline constexpr auto EXAT = "exat";
    inline constexpr auto EXPIREAT = "expireat";
}

namespace Redis::Util {
    using HashMap = std::unordered_map<std::string, std::string>;
    using StringSet = std::unordered_set<std::string>;
    using SecondsOpt = std::optional<std::chrono::seconds>;

    inline HashMap ParseHashMap(const Database::Redis::UniqueReply& reply) {
        HashMap hash_map;
        hash_map.reserve(reply->elements);
        for (int i = 0; i < reply->elements; i += 2) {
            const char* key = reply->element[i]->str;
            const char* val = reply->element[i + 1]->str;
            hash_map[key] = val;
        }
        return hash_map;
    }

    inline std::unordered_set<std::string> ParseStringSet(const Database::Redis::UniqueReply& reply) {
        std::unordered_set<std::string> set;
        set.reserve(reply->elements);
        for (int i = 0; i < reply->elements; ++i) {
            if (reply->element[i]->type == REDIS_REPLY_STRING)
                set.emplace(reply->element[i]->str);
        }
        return set;
    }
}
