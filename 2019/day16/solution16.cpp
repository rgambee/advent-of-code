// Solution based on https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm

#include <iostream>
#include <valarray>
#include <cmath>
#include <stdexcept>
#include <string>

#include "utils.h"


constexpr auto NUM_PHASES = 1;//100;
constexpr auto REPEATS = 1;//10000;
const std::valarray<int> BASE_PATTERN{0, 1, 0, -1};


// This could be optimized to reduce the amount of copying that happens
std::valarray<int> fft(const std::valarray<int> &input_list) {
    auto N = input_list.size();
    std::valarray<int> output_list(N);
    if (N == 1) {
        return input_list;
    }

    output_list[std::slice(0, N / 2, 1)] = fft(input_list[std::slice(0, N/2, 2)]);
    output_list[std::slice(N / 2, N / 2, 1)] = fft(input_list[std::slice(1, N/2, 2)]);
    for (size_t k = 0; k < N / 2; ++k) {
        auto temp = output_list[k];
        auto twiddle = BASE_PATTERN[((k + 1) * BASE_PATTERN.size() / N) % BASE_PATTERN.size()];
        output_list[k] = std::abs((temp + twiddle * output_list[k + N / 2]) % 10);
        output_list[k + N / 2] = std::abs((temp - twiddle * output_list[k + N / 2]) % 10);
    }
    return output_list;
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::string line;
    std::getline(input_stream, line);
    std::valarray<int> list(line.size() * REPEATS);
    for (size_t i = 0; i < REPEATS; ++i) {
        for (size_t j = 0; j < line.size(); ++j) {
            list[i * line.size() + j] = line[j] - '0';
        }
    }

    auto log = std::log2(list.size());
    if (log != std::floor(log)) {
        throw std::runtime_error(
            "Input list length is not a power of 2");
    }

    for (auto phase = 0; phase < NUM_PHASES; ++phase) {
        list = fft(list);
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "First 8 digits of final list: ";
    for (int i = 0; i < 8; ++i) {
        std::cout << list[i];
    }
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
