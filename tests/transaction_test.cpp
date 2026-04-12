//
// Created by Shinnosuke Kawai on 4/5/26.
//
#include "suites/transaction_suite.h"
// TEST_F(TransactionTest, FutureBasedTxn) {
//     CREATE_CLIENT(m_config);
//     auto txn_res = client.perform_transaction([](rediscxx::transaction& txn) {
//         txn.watch("key1");
//         txn.watch("key2");
//         txn.enqueue(rediscxx::command::set, "key1", "value1");
//         txn.enqueue(rediscxx::command::set, "key2", "value2");
//         txn.enqueue(rediscxx::command::set, "key3", "value3");
//         txn.enqueue(rediscxx::command::set, "key4", "value4");
//         txn.enqueue(rediscxx::command::set, "key5", "value5");
//     }).get();
//     timer.print_elapsed_time();
//     ASSERT_TRUE(txn_res.has_value());
//     auto txn = std::move(txn_res.value());
//     ASSERT_TRUE(txn.has_value());
// }
//
//
// TEST_F(TransactionTest, CallbackBasedTxn) {
//     CREATE_CLIENT(m_config);
//     auto promise = std::make_shared<std::promise<void>>();
//     client.perform_transaction_async([](rediscxx::transaction& txn) {
//         txn.watch("key1");
//         txn.watch("key2");
//         txn.enqueue(rediscxx::command::set, "key1", "value1");
//         txn.enqueue(rediscxx::command::set, "key2", "value2");
//         txn.enqueue(rediscxx::command::set, "key3", "value3");
//         txn.enqueue(rediscxx::command::set, "key4", "value5");
//         txn.enqueue(rediscxx::command::set, "key5", "value6");
//     },
//     nullptr,
//     [promise](const rediscxx::exception& err) {
//         FAIL() << err.to_str();
//     });
//
//     std::println("Waiting for transaction to complete");
//     std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//     timer.print_elapsed_time();
// }
int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
