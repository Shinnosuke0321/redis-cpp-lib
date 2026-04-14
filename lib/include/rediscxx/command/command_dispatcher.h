//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "base_command.h"

namespace rediscxx {

    class command_dispatcher {
        using command_result_fn = std::function<void(std::expected<result::reply, error::redis_exception>)>;
    public:
        command_dispatcher(const command cmd, internal::arg_buffer&& buffer, command_result_fn&& handler)
        : m_cmd(cmd), m_buffer(std::move(buffer)), m_handler(std::move(handler)) {}

        template<typename T>
        requires (std::derived_from<T, command_base> && std::derived_from<T, core::ref_counted<T>>)
        void dispatch(std::function<void(smart_ptr::intrusive_ptr<T>)> &&fn) {
            if (m_cmd == T::get_static_type()) {
                fn(smart_ptr::make_intrusive<T>(std::move(m_buffer), std::move(m_handler)));
            }
        }
    private:
        command m_cmd;
        internal::arg_buffer m_buffer;
        std::function<void(std::expected<result::reply, error::redis_exception>)> m_handler;
    };
}