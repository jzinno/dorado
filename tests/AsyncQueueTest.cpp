#include "utils/AsyncQueue.h"

#include <catch2/catch.hpp>

#define TEST_GROUP "AsyncQueue "

#include <atomic>
#include <iostream>
#include <thread>

TEST_CASE(TEST_GROUP ": InputsMatchOutputs") {
    const int n = 10;
    AsyncQueue<int> queue(n);

    for (int i = 0; i < n; ++i) {
        const bool success = queue.try_push(std::move(i));
        REQUIRE(success);
    }
    for (int i = 0; i < n; ++i) {
        int val = -1;
        const bool success = queue.try_pop(val);
        REQUIRE(val == i);
    }
}

TEST_CASE(TEST_GROUP ": PushFailsIfTerminating") {
    AsyncQueue<int> queue(1);
    queue.terminate();
    const bool success = queue.try_push(42);
    REQUIRE(!success);
}

TEST_CASE(TEST_GROUP ": PopFailsIfTerminating") {
    AsyncQueue<int> queue(1);
    queue.terminate();
    int val;
    const bool success = queue.try_pop(val);
    REQUIRE(!success);
}

// Spawned thread sits waiting for an item.
// Main thread supplies that item.
TEST_CASE(TEST_GROUP ": PopFromOtherThread") {
    AsyncQueue<int> queue(1);
    std::atomic_bool thread_started{false};
    bool try_pop_result = false;

    auto popping_thread = std::thread([&]() {
        thread_started.store(true, std::memory_order_relaxed);
        int val = -1;
        // catch2 isn't thread safe so we have to check this on the main thread
        try_pop_result = queue.try_pop(val);
    });

    // Wait for thread to start
    while (!thread_started.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Feed data to the thread
    const bool success = queue.try_push(42);
    REQUIRE(success);

    popping_thread.join();
    REQUIRE(try_pop_result);
}

// Spawned thread sits waiting for an item.
// Main thread terminates wait.
TEST_CASE(TEST_GROUP ": TerminateFromOtherThread") {
    AsyncQueue<int> queue(1);
    std::atomic_bool thread_started{false};
    bool try_pop_result = false;

    auto popping_thread = std::thread([&]() {
        thread_started.store(true, std::memory_order_relaxed);
        int val = -1;
        // catch2 isn't thread safe so we have to check this on the main thread
        try_pop_result = queue.try_pop(val);
    });

    // Wait for thread to start
    while (!thread_started.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop it
    queue.terminate();
    popping_thread.join();

    // This will fail, since the wait is terminated.
    REQUIRE(!try_pop_result);
}