//
// Created by Shinnosuke Kawai on 3/28/26.
//
#include "rediscxx/client.h"
#include "rediscxx/transaction.h"
#include <atomic>
#include <thread>

namespace rediscxx {
    std::future<std::expected<std::optional<std::deque<result::reply>>, exception>> client::perform_transaction(transaction_executor&& executor) const noexcept {
        transaction txn(this, transaction::passkey{});
        executor(txn);
        return txn.exec();
    }

    void client::perform_transaction_async(transaction_executor &&executor, std::function<void(std::optional<std::deque<result::reply>>)> &&success_cb, std::function<void(exception)> &&error_cb) const noexcept {
        transaction txn(this, transaction::passkey{});
        executor(txn);
        txn.exec(std::move(success_cb), std::move(error_cb));
    }

    std::future<std::expected<std::optional<std::deque<result::reply>>, exception>> transaction::exec() noexcept {
        using result_t = std::expected<std::optional<std::deque<result::reply>>, exception>;
        auto promise = std::make_shared<std::promise<result_t>>();
        auto future = promise->get_future();
        auto aborted = std::make_shared<std::atomic<bool>>(false);

        auto on_intermediate_error = [promise, aborted](const exception& e) {
            if (bool expected = false; aborted->compare_exchange_strong(expected, true)) {
                promise->set_value(std::unexpected(e));
            }
        };

        // WATCH
        for (auto& key : m_watch_keys) {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::watch), key);
            single_request req(std::move(buf),nullptr,on_intermediate_error);
            m_client->Handle(req);
        }

        // MULTI
        {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::multi));
            single_request req(std::move(buf),nullptr,on_intermediate_error);
            m_client->Handle(req);
        }

        // Queued commands — discard "+QUEUED" replies
        while (!m_queued_commands.empty()) {
            single_request req = std::move(m_queued_commands.front());
            m_queued_commands.pop_front();
            req.on_success = [](result::reply) {};
            req.on_error   = on_intermediate_error;
            m_client->Handle(req);
        }

        // EXEC
        {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::exec));
            single_request req(std::move(buf),
                [promise, aborted](const result::reply &r) {
                    if (aborted->load())
                        return;
                    if (r.is_nil()) {
                        promise->set_value(std::unexpected(exception::TransactionAborted()));
                        return;
                    }
                    if (auto replies = r.as_replies()) {
                        promise->set_value(std::move(*replies));
                    } else {
                        promise->set_value(std::unexpected(exception::ReplyError("exec reply is not an array")));
                    }
                },
                [promise, aborted](const exception& e) {
                    if (bool expected = false; aborted->compare_exchange_strong(expected, true)) {
                        promise->set_value(std::unexpected(e));
                    }
                });
            m_client->Handle(req);
        }

        return future;
    }

    void transaction::exec(std::function<void(std::optional<std::deque<result::reply>>)> &&success_cb, std::function<void(exception)> &&error_cb) noexcept {
        auto aborted = std::make_shared<std::atomic<bool>>(false);
        auto shared_on_err = std::make_shared<std::function<void(exception)>>(std::move(error_cb));
        auto on_intermediate_error = [aborted, shared_on_err](const exception& e) {
            if (bool expected = false; aborted->compare_exchange_strong(expected, true)) {
                if (*shared_on_err) (*shared_on_err)(e);
            }
        };

        // WATCH
        for (auto& key : m_watch_keys) {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::watch), key);
            single_request req(std::move(buf),nullptr,on_intermediate_error);
            m_client->Handle(req);
        }

        // MULTI
        {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::multi));
            single_request req(std::move(buf),nullptr,on_intermediate_error);
            m_client->Handle(req);
        }

        // Queued commands — discard "+QUEUED" replies
        while (!m_queued_commands.empty()) {
            auto req = std::move(m_queued_commands.front());
            m_queued_commands.pop_front();
            req.on_error   = on_intermediate_error;
            m_client->Handle(req);
        }

        // EXEC
        {
            internal::arg_buffer buf = internal::to_args(command_to_string(command::exec));
            single_request req(std::move(buf),
                [aborted, on_success = std::move(success_cb), shared_on_err](const result::reply &r) {
                    if (aborted->load())
                        return;
                    if (r.is_nil()) {
                        if (*shared_on_err) (*shared_on_err)(exception::TransactionAborted());
                        return;
                    }
                    if (auto replies = r.as_replies()) {
                        if (on_success) on_success(std::move(*replies));
                    } else {
                        if (*shared_on_err) (*shared_on_err)(exception::ReplyError("exec reply is not an array"));
                    }
                },
                [aborted, shared_on_err](const exception& e) {
                    if (bool expected = false; aborted->compare_exchange_strong(expected, true)) {
                        if (*shared_on_err) (*shared_on_err)(e);
                    }
                });
            m_client->Handle(req);
        }
    }
}
