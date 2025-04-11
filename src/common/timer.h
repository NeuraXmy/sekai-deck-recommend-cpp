#include <chrono>
#include <iostream>
#include <string>


class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string name;

public:

    Timer(const std::string& timer_name = "") : name(timer_name) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    long long elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        return static_cast<long long>(duration);
    }

    void print(bool reset = true) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        std::cout << name << " took " << duration / 1000. << " ms." << std::endl;
        if (reset) {
            start_time = std::chrono::high_resolution_clock::now();
        }
    }
};