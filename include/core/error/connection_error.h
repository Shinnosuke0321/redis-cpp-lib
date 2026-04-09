//
// Created by Shinnosuke Kawai on 4/10/26.
//
#pragma once
#include <string_view>
#include "base_error.h"

namespace core {
    enum class connection_errc {
        ConnectionFailed,
        MissingConfig,
        FactoryNotRegistered,
        Timeout,
        SocketFailed,
        AuthFailed
    };

    class connection_error: public typed_error<connection_error, connection_errc> {
    public:
        ERROR_CATEGORY_NAME(connection)

        static std::string_view code_to_string(const connection_errc e) noexcept {
            switch (e) {
                case connection_errc::ConnectionFailed: return "ConnectionFailed";
                case connection_errc::MissingConfig: return "MissingConfig";
                case connection_errc::FactoryNotRegistered: return "FactoryNotRegistered";
                case connection_errc::Timeout: return "Timeout";
                case connection_errc::SocketFailed: return "SocketFailed";
                case connection_errc::AuthFailed: return "AuthFailed";
            }
            return "Unknown";
        }
    };

#define CONNECTION_ERROR(type, message) \
    core::connection_error{core::connection_errc::type, (message)}

}
