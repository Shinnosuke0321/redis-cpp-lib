//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "set_command.h"
#include "get_command.h"

namespace rediscxx {
    inline smart_ptr::intrusive_ptr<set_command> make_set_command(internal::arg_buffer &&args, std::function<void(std::expected<result::reply, error::redis_exception>)> &&handler) {
        return smart_ptr::make_intrusive<set_command>(std::move(args), std::move(handler));
    }

    inline smart_ptr::intrusive_ptr<get_command> make_get_command(internal::arg_buffer &&args, std::function<void(std::expected<result::reply, error::redis_exception>)> &&handler) {
        return smart_ptr::make_intrusive<get_command>(std::move(args), std::move(handler));
    }
}
