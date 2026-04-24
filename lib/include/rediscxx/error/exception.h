//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string_view>
#include <core/error/base_error.h>

namespace rediscxx::error {
    enum class types {
        Unknown = 0,
        CommandFailed, CallbackRegistrationFailed, EventLoopAttachFailed, AsyncConnectFailed,
        IO, EndOfFile, ProtocolErr, OutOfMemory, Timeout, Other,
    };

    class redis_exception: public core::error::typed_error<redis_exception, types> {
    public:
        ERROR_CLASS_CATEGORY(Redis)
        COPY_SEMANTICS(redis_exception, default);
        MOVE_SEMANTICS(redis_exception, default);
        ~redis_exception() override = default;
    };

    inline redis_exception from_ctx(const redisAsyncContext* ctx) noexcept {
        using types::IO, types::EndOfFile, types::ProtocolErr, types::OutOfMemory, types::Timeout, types::Other, types::Unknown;

        auto msg_from = [](const char* str, const std::string_view def_val) -> std::string {
            return str ? str : std::string(def_val);
        };
        if (!ctx) {
            return CREATE_ERROR(redis_exception, Unknown, "ctx is null");
        }
        switch (ctx->err) {
            case REDIS_ERR_IO:
                return CREATE_ERROR(redis_exception, IO, msg_from(ctx->errstr, "Error in read or write"));
            case REDIS_ERR_EOF:
                return CREATE_ERROR(redis_exception, EndOfFile, msg_from(ctx->errstr, "Server closed the connection"));
            case REDIS_ERR_OTHER:
                return CREATE_ERROR(redis_exception, Other, msg_from(ctx->errstr, "Unknown error"));
            case REDIS_ERR_PROTOCOL:
                return CREATE_ERROR(redis_exception, ProtocolErr, msg_from(ctx->errstr, "Protocol error"));
            case REDIS_ERR_TIMEOUT:
                return CREATE_ERROR(redis_exception, Timeout, msg_from(ctx->errstr, "Timeout"));
            case REDIS_ERR_OOM:
                return CREATE_ERROR(redis_exception, OutOfMemory, msg_from(ctx->errstr, "Out of memory"));
            default:
                return CREATE_ERROR(redis_exception, Unknown, msg_from(ctx->errstr, "Unknown error"));
        }
    }
}
