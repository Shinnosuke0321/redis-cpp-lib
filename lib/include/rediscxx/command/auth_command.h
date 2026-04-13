//
// Created by Shinnosuke Kawai on 4/12/26.
//
#pragma once
#include "rediscxx/error/exception.h"
#include "base_command.h"

namespace rediscxx {
    class auth_command: public command_base, public core::ref_counted<auth_command> {
    public:
        explicit auth_command(std::string password, std::function<void(std::expected<void, error::redis_exception>)>&& handler)
        : m_password(std::move(password)),
          m_handler(std::move(handler)) {}
    public:
        COMMAND_CLASS_TYPE(auth)

        void execute(redisAsyncContext *ctx) override {
            using error::types::AuthCommand;
            if (auto self = this->intrusive_from_this(); redisAsyncCommand(ctx, on_callback, new smart_ptr::intrusive_ptr(std::move(self)), "AUTH %s", m_password.c_str()) != REDIS_OK) {
                m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, AuthCommand, "failed to send auth"));
            }
        }

    private:
        static void on_callback(redisAsyncContext *ctx, void *r, void *priv) {
            using error::types::AuthCommand;
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<auth_command>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<auth_command>*>(priv);
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, AuthCommand, reply->str));
            }
            else {
                self->m_handler({});
            }
        }
    private:
        std::string m_password;
        std::function<void(std::expected<void, error::redis_exception>)> m_handler;
    };
}