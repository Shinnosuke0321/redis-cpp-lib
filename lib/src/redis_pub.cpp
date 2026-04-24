//
// Created by Shinnosuke Kawai on 11/3/25.
//

#include "rediscxx/client.h"

namespace rediscxx {
    client::~client() = default;

    std::expected<void, core::error::exception> client::connect() const noexcept {
        using error_variant = std::variant<event::event_loop_error, error::redis_exception>;
        std::promise<std::expected<void, core::error::exception>> promise;
        auto future = promise.get_future();
        m_transport->connect_async(
            m_config.host,
            m_config.port,
            m_config.password,
            m_config.dbIndex,
            [&promise](std::expected<void, error_variant> res) {
                if (!res) {
                    std::visit([&promise](auto&& err) {
                        promise.set_value(std::unexpected(std::move(err.to_exception())));
                    }, res.error());
                } else {
                    promise.set_value({});
                }
            });
        return future.get();
    }

}
