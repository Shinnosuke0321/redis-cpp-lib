//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string_view>
#include <core/error/base_error.h>

namespace rediscxx::error {
    enum class code {
        Unknown = 0,
        NullReply, Command,
        CallbackRegistrationFailed, EventLoopAttachFailed, AsyncConnectFailed,
        IO, EndOfFile, ProtocolErr, OutOfMemory, Timeout, Other,
    };

    class redis_exception: public core::error::typed_error<redis_exception, code> {
    public:
        ERROR_CLASS_CATEGORY(Redis)
        ~redis_exception() override = default;
    };

    inline std::unexpected<redis_exception> from_ctx(const redisAsyncContext* ctx) noexcept {
        using code::IO, code::EndOfFile, code::ProtocolErr, code::OutOfMemory, code::Timeout, code::Other, code::Unknown;

        auto msg_from = [](const char* str, const std::string_view def_val) -> std::string {
            return str ? str : std::string(def_val);
        };
        if (!ctx) {
            return MAKE_UNEXPECTED_ERROR(redis_exception, Unknown, "ctx is null");
        }
        switch (ctx->err) {
            case REDIS_ERR_IO:
                return MAKE_UNEXPECTED_ERROR(redis_exception, IO, msg_from(ctx->errstr, "Error in read or write"));
            case REDIS_ERR_EOF:
                return MAKE_UNEXPECTED_ERROR(redis_exception, EndOfFile, msg_from(ctx->errstr, "Server closed the connection"));
            case REDIS_ERR_OTHER:
                return MAKE_UNEXPECTED_ERROR(redis_exception, Other, msg_from(ctx->errstr, "Unknown error"));
            case REDIS_ERR_PROTOCOL:
                return MAKE_UNEXPECTED_ERROR(redis_exception, ProtocolErr, ctx->errstr);
            case REDIS_ERR_TIMEOUT:
                return MAKE_UNEXPECTED_ERROR(redis_exception, Timeout, ctx->errstr);
            case REDIS_ERR_OOM:
                return MAKE_UNEXPECTED_ERROR(redis_exception, OutOfMemory, ctx->errstr);
            default:
                return MAKE_UNEXPECTED_ERROR(redis_exception, Unknown, ctx->errstr);
        }
    }
}