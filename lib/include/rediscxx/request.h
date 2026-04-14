//
// Created by Shinnosuke Kawai on 3/28/26.
//

#pragma once
#include "result/reply.h"
#include "internal/arg_parser.h"
#include "exception.h"
#include <functional>

namespace rediscxx {
    using result_callback = std::function<void(result::reply)>;
    using error_callback = std::function<void(const exception &)>;

    struct single_request {
        result_callback on_success = nullptr;
        error_callback on_error = nullptr;

        const internal::arg_buffer& get_arg_buffer() const noexcept {
            return arg_buffer;
        }

        single_request() = delete;
        explicit single_request(internal::arg_buffer&& arg_buffer_) noexcept
        : arg_buffer(std::move(arg_buffer_)) {
        }

        single_request(internal::arg_buffer&& arg_buffer, result_callback&& on_success, error_callback&& on_error) noexcept
        : on_success(std::move(on_success)),
          on_error(std::move(on_error)),
          arg_buffer(std::move(arg_buffer)){
        }

        single_request(single_request&& other) noexcept
        : on_success(std::move(other.on_success)),
          on_error(std::move(other.on_error)),
          arg_buffer(std::move(other.arg_buffer)) {
            other.on_success = nullptr;
            other.on_error = nullptr;
        }

        single_request& operator=(single_request&& other) noexcept {
            if (this != &other) {
                arg_buffer = std::move(other.arg_buffer);
                on_success = std::move(other.on_success);
                on_error = std::move(other.on_error);
                other.on_success = nullptr;
                other.on_error = nullptr;
            }
            return *this;
        }

        single_request(const single_request&) = delete;
        single_request& operator=(const single_request&) = delete;
        ~single_request() = default;
    private:
        internal::arg_buffer arg_buffer;
    };
}
