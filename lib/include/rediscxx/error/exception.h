//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string_view>
#include <core/error/base_error.h>

namespace rediscxx::error {
    enum class error_code {
        null_reply, command_error, network_error, timeout,
        CallbackRegistrationFailed, EventLoopAttachFailed
    };

    class redis_exception: public core::error::typed_error<redis_exception, error_code> {
    public:
        ERROR_CLASS_CATEGORY(redis)
        ~redis_exception() override = default;
    };
}