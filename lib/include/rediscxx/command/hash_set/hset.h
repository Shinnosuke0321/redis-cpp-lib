//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "rediscxx/command/base_command.h"

namespace rediscxx {
    class hset_command : public command_base, public core::ref_counted<hset_command> {
    public:
        using on_callback = std::function<void(std::expected<result::reply, error::redis_exception>)>;
        COMMAND_CLASS_TYPE(hset)

    public:
        hset_command(internal::arg_buffer&& args, on_callback&& handler): m_args(std::move(args)), m_handler(std::move(handler)) {
            m_args.append_at_front(hset_command::to_string());
        }

        void execute(redisAsyncContext *ctx) override {
            const int argc = m_args.argc();
            std::vector<const char*> argv = m_args.argv();
            std::vector<size_t> argvlen = m_args.argvlen();
            if (auto intrusive_self = this->intrusive_from_this(); redisAsyncCommandArgv(ctx, on_handle, new smart_ptr::intrusive_ptr(std::move(intrusive_self)), argc, argv.data(), argvlen.data()) != REDIS_OK) {
                m_handler(std::unexpected(error::from_ctx(ctx)));
            }
        }
    private:
        static void on_handle(redisAsyncContext *ctx, void *r, void *priv) noexcept {
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<hset_command>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<hset_command>*>(priv);
            using error::types::HSetCommand;
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, HSetCommand, reply->str));
            }
            else {
                self->m_handler(result::reply::from_raw(reply));
            }
        }
    private:
        internal::arg_buffer m_args;
        on_callback m_handler;
    };
}