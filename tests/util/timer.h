//
// Created by Shinnosuke Kawai on 4/2/26.
//
#pragma once
#include <print>
#include <chrono>

class Timer {
public:
    // Constructor starts the timer immediately
    Timer() : start_point_(std::chrono::steady_clock::now()) {}

    // Restart the timer
    void reset() {
        start_point_ = std::chrono::steady_clock::now();
    }

    // Get the elapsed time in milliseconds
    double elapsed_milliseconds() const {
        auto end_point = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_point - start_point_);
        return static_cast<double>(duration.count());
    }

    // Get the elapsed time in seconds
    double elapsed_seconds() const {
        auto end_point = std::chrono::steady_clock::now();
        std::chrono::duration<double> duration = end_point - start_point_;
        return duration.count();
    }

    void print_elapsed_time() const {
        std::println("Elapsed time: {} ms", elapsed_milliseconds());
    }

private:
    std::chrono::steady_clock::time_point start_point_;
};
