#include <chrono>
#include <thread>
#include <iostream>
#include <format>

uint64_t rdtsc() {
    return __builtin_ia32_rdtsc();
}

uint64_t average(uint64_t low, uint64_t high) {
    return low + (high - low) / 2;
}

std::tuple<uint64_t, std::chrono::steady_clock::time_point> get_tsc_ns_pair() {
    constexpr int try_times = 5;
    uint64_t tsc_arr[try_times + 1];
    std::chrono::steady_clock::time_point ns_arr[try_times];

    tsc_arr[0] = rdtsc();
    for (int i = 0; i < try_times; ++i) {
        ns_arr[i] = std::chrono::steady_clock::now();
        tsc_arr[i + 1] = rdtsc();
    }

    auto min_tsc_diff = tsc_arr[1] - tsc_arr[0];
    auto ave_tsc = average(tsc_arr[0], tsc_arr[1]);
    auto ns = ns_arr[0];

    for (int i = 1; i < try_times; ++i) {
        auto tsc_diff = tsc_arr[i + 1] - tsc_arr[i];
        if (tsc_diff < min_tsc_diff) {
            min_tsc_diff = tsc_diff;
            ave_tsc = average(tsc_arr[i], tsc_arr[i + 1]);
            ns = ns_arr[i];
        }
    }

    return {ave_tsc, ns};
}

int main() {
    auto [base_tsc, base_ns] = get_tsc_ns_pair();
    auto n = 1;
    double sum_x = 0., sum_y = 0., sum_xx = 0., sum_xy = 0.;
    while (true) {
        using namespace std::chrono_literals;

        std::this_thread::sleep_for(1s);
        auto [tsc, ns] = get_tsc_ns_pair();
        auto tsc_diff = static_cast<double>(tsc - base_tsc);
        auto ns_diff = static_cast<double>((ns - base_ns).count());
        ++n;
        sum_x += ns_diff;
        sum_y += tsc_diff;
        sum_xy += ns_diff * tsc_diff;
        sum_xx += ns_diff * ns_diff;
        double divisor = n * sum_xx - sum_x * sum_x;
        double a = (n * sum_xy - sum_x * sum_y) / divisor;
        double b = (sum_xx * sum_y - sum_x * sum_xy) / divisor;
        std::cout << std::format("{:.16}\n", a);
    }
}