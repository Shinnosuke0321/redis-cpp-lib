//
// Created by Shinnosuke Kawai on 4/14/26.
//

#pragma once
#include "../base_command.h"
#include "rediscxx/error/exception.h"

namespace rediscxx {
    class hget_command : public command_base, public core::ref_counted<hget_command> {
        using on_handler = std::function<void(std::expected<result::reply, error::redis_exception>)>;
    public:
        COMMAND_CLASS_TYPE(hget);
        explicit hget_command(internal::arg_buffer&& args, on_handler&& handler)
        : m_args(std::move(args)), m_handler(std::move(handler)) {
            m_args.append_at_front(hget_command::to_string());
        }
    public:
        void execute(redisAsyncContext *ctx) override {
            const auto argc = m_args.argc();
            auto argv = m_args.argv();
            auto argvlen = m_args.argvlen();
            if (auto intrusive_self = this->intrusive_from_this(); redisAsyncCommandArgv(ctx, on_handle, new smart_ptr::intrusive_ptr(std::move(intrusive_self)), argc, argv.data(), argvlen.data()) != REDIS_OK) {
                m_handler(std::unexpected(error::from_ctx(ctx)));
            }
        }
    private:
        static void on_handle(redisAsyncContext* ctx, void* r, void* priv) noexcept {
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<hget_command>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<hget_command>*>(priv);
            using error::types::HGetCommand;
            if (const auto* reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(MAKE_UNEXPECTED_ERROR(error::redis_exception, HGetCommand, reply->str));
            }
            else {
                self->m_handler(result::reply::from_raw(reply));
            }
        }

    private:
        internal::arg_buffer m_args;
        on_handler m_handler = nullptr;
    };
}