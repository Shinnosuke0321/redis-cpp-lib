//
// Created by Shinnosuke Kawai on 4/12/26.
//
#pragma once
#include "rediscxx/error/exception.h"
#include "base_command.h"

namespace rediscxx {
    class select_command: public command_base, public core::ref_counted<select_command> {
    public:
        explicit select_command(const int index, std::function<void(std::expected<void, error::redis_exception>)>&& handler)
        : m_index(index),
          m_handler(std::move(handler)){}
    public:
        COMMAND_CLASS_TYPE(select)

        void execute(redisAsyncContext *ctx) override {
            using namespace rediscxx::error;
            using types::SelectCommand;
            if (auto self = this->intrusive_from_this();
                redisAsyncCommand(ctx, on_callback, new smart_ptr::intrusive_ptr(std::move(self)), "SELECT %d", m_index) != REDIS_OK) {
                m_handler(MAKE_UNEXPECTED_ERROR(redis_exception, SelectCommand, "failed to send select"));
                }
        }
        ~select_command() override = default;
    private:
        static void on_callback(redisAsyncContext *ctx, void *r, void *priv) {
            using error::types::SelectCommand;

            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<select_command>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<select_command>*>(priv);
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, SelectCommand, reply->str));
            }
            else {
                self->m_handler({});
            }
        }

    private:
        int m_index;
        std::function<void(std::expected<void, error::redis_exception>)> m_handler;
    };
}