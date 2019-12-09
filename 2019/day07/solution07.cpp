#include <algorithm>
#include <array>
#include <chrono>
#include <future>
#include <limits>
#include <thread>

#include "utils.h"


using phase_settings_type = std::array<int, 5>;


int simulate_phase_settings(std::vector<int> program,
                            const phase_settings_type &phase_settings) {
    std::vector<std::promise<int> > promises(phase_settings.size());
    std::vector<std::future<int> > async_futures;
    for (phase_settings_type::size_type i = 0; i < phase_settings.size(); ++i) {
        promises[i].set_value(phase_settings[i]);

        auto get_input = [&promises, i]() -> int {
            auto input = promises[i].get_future().get();
            // Reset promise so it can be used later
            promises[i] = std::promise<int>();
            return input;
        };

        auto give_output = [&promises, i](int output) -> void {
            auto ind = (i + 1) % promises.size();
            promises[ind].set_value(output);
        };

        async_futures.push_back(std::move(std::async(
            std::launch::async,
            // Wrap run_intcode_program in lambda so overload resolution works
            [program, get_input, give_output]() -> int {
                return run_intcode_program(program, get_input, give_output);
            })));
    }
    // Wait a bit for the first thread to be ready for the next input
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    promises[0].set_value(0);
    async_futures[phase_settings.size() - 1].wait();
    return promises[0].get_future().get();
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    std::cout << "Calculating part 1..." << std::endl;
    phase_settings_type phase_settings{0, 1, 2, 3, 4};
    auto part1_max = std::numeric_limits<int>::min();
    do {
        auto signal = simulate_phase_settings(program, phase_settings);
        part1_max = std::max(part1_max, signal);
    } while (std::next_permutation(phase_settings.begin(), phase_settings.end()));

    std::cout << "Calculating part 2..." << std::endl;
    phase_settings = phase_settings_type{5, 6, 7, 8, 9};
    auto part2_max = std::numeric_limits<int>::min();
    do {
        auto signal = simulate_phase_settings(program, phase_settings);
        part2_max = std::max(part2_max, signal);
    } while (std::next_permutation(phase_settings.begin(), phase_settings.end()));


    std::cout << "PART 1" << std::endl;
    std::cout << "Max thruster signal: " << part1_max << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Max thruster signal: " << part2_max << std::endl;
    return 0;
}
