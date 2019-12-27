#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "intcode.h"
#include "utils.h"


using phase_settings_type = std::array<intcode_type, 5>;


// This is very slow. Maybe running the intcode program is taking a long time?
// Maybe all the thread-switching is adding a lot of overhead?
intcode_type simulate_phase_settings(program_type program,
                                     const phase_settings_type &phase_settings) {
    MultiQueue<intcode_type> mq;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < phase_settings.size(); ++i) {
        mq.push_queue(i, phase_settings[i]);

        auto get_input = [&mq, i]() -> intcode_type {
            return mq.pop_queue(i);
        };

        auto size = phase_settings.size();
        auto give_output = [&mq, i, size](intcode_type output) -> void {
            mq.push_queue((i + 1) % size, output);
        };

        threads.emplace_back(
            // Wrap run_intcode_program in lambda so overload resolution works
            [program, get_input, give_output]() -> intcode_type {
                return run_intcode_program(program, get_input, give_output);
            });
    }
    // Set input signal of 0 to first thruster to start
    mq.push_queue(0, 0);
    // Wait for all threads to finish
    for (auto& thd: threads) {
        thd.join();
    }
    // Return final output from last thruster
    return mq.pop_queue(0);
}


template <typename T>
T factorial(T num) {
    return (num == 0) ? 1 : num * factorial(num - 1);
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    phase_settings_type phase_settings{0, 1, 2, 3, 4};
    auto part1_max = std::numeric_limits<intcode_type>::min();
    auto iteration = 0;
    do {
        std::cout << "Calculating part 1: " << iteration;
        std::cout << " / " << factorial(phase_settings.size()) << '\r';
        std::flush(std::cout);
        auto signal = simulate_phase_settings(program, phase_settings);
        part1_max = std::max(part1_max, signal);
        ++iteration;
    } while (std::next_permutation(phase_settings.begin(), phase_settings.end()));
    std::cout << std::endl;

    phase_settings = phase_settings_type{5, 6, 7, 8, 9};
    auto part2_max = std::numeric_limits<intcode_type>::min();
    iteration = 0;
    do {
        std::cout << "Calculating part 2: " << iteration;
        std::cout << " / " << factorial(phase_settings.size()) << '\r';
        std::flush(std::cout);
        auto signal = simulate_phase_settings(program, phase_settings);
        part2_max = std::max(part2_max, signal);
        ++iteration;
    } while (std::next_permutation(phase_settings.begin(), phase_settings.end()));
    std::cout << std::endl;


    std::cout << "PART 1" << std::endl;
    std::cout << "Max thruster signal: " << part1_max << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Max thruster signal: " << part2_max << std::endl;
    return 0;
}
