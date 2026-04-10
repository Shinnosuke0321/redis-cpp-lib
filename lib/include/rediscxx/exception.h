//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string_view>
#include <core/error/base_error.h>

namespace rediscxx {
    enum class error_code {
        null_reply, command_error, network_error, timeout
    };

    class redis_exception: public core::typed_error<redis_exception, error_code> {
    public:
        ERROR_CATEGORY_NAME(redis);
        static std::string_view code_to_string(const error_code e) noexcept {
            switch (e) {
                case error_code::null_reply: return "Null reply";
                case error_code::command_error: return "Command error";
                case error_code::network_error: return "Network error";
                case error_code::timeout: return "Timeout";
            }
            return "Unknown";
        }
    };
#define REDIS_ERROR()
}