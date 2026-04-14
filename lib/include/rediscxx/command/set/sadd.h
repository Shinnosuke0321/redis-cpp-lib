//
// Created by Shinnosuke Kawai on 4/14/26.
//

#pragma once
#include "rediscxx/command/base_command.h"

namespace rediscxx {
    class sadd_command: public command_base, public core::ref_counted<sadd_command> {
    public:
        using on_callback = std::function<void(std::expected<result::reply, error::redis_exception>)>;
    public:
        COMMAND_CLASS_TYPE(sadd)

        explicit sadd_command(internal::arg_buffer&& args, on_callback&& handler)
        : m_args(std::move(args)), m_handler(std::move(handler)) {
            m_args.append_at_front(sadd_command::to_string());
        }

        void execute(redisAsyncContext *ctx) override {
            const auto argc = m_args.argc();
            auto argv = m_args.argv();
            auto argvlen = m_args.argvlen();
            if (auto intrusive_self = this->intrusive_from_this();
                redisAsyncCommandArgv(ctx, on_handle, new smart_ptr::intrusive_ptr(std::move(intrusive_self)), argc, argv.data(), argvlen.data()) != REDIS_OK) {
                m_handler(std::unexpected(error::from_ctx(ctx)));
                }
        }
    private:
        static void on_handle(redisAsyncContext *ctx, void *r, void *priv) noexcept {
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<sadd_command>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<sadd_command>*>(priv);
            using error::types::SetCommand;
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, SetCommand, reply->str));
            }
            else {
                self->m_handler(result::reply::from_raw(reply));
            }
        }
    private:
        internal::arg_buffer m_args;
        std::function<void(std::expected<result::reply, error::redis_exception>)> m_handler = nullptr;
    };;
}