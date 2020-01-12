#include <iostream>
#include <valarray>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

#include "utils.h"


constexpr auto NUM_PHASES = 100;
constexpr auto REPEATS = 10000;
constexpr auto OFFSET_LENGTH = 7;
constexpr auto OUTPUT_LENGTH = 8;
const std::valarray<int> BASE_PATTERN{0, 1, 0, -1};


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::string line;
    std::getline(input_stream, line);
    std::valarray<int> list(line.size());
    for (std::string::size_type i = 0; i < line.size(); ++i) {
        list[i] = line[i] - '0';
    }
    // Save a copy of the original list for later
    std::valarray<int> original_list(list);

    // Solve part 1 using the naive approach
    std::valarray<int> pattern(list.size());
    std::valarray<int> next_list(list.size());
    for (auto phase = 0; phase < NUM_PHASES; ++phase) {
        for (size_t output_index = 0; output_index < list.size(); ++output_index) {
            for (size_t i = 0; i < pattern.size(); ++i) {
                auto base_index = ((i + 1) / (output_index + 1) % BASE_PATTERN.size());
                pattern[i] = BASE_PATTERN[base_index];
            }

            next_list[output_index] = std::abs((list * pattern).sum()) % 10;
        }
        list = next_list;
    }
    auto part1_answer = 0;
    for (auto i = 0; i < OUTPUT_LENGTH; ++i) {
        part1_answer = part1_answer * 10 + list[i];
    }

    // For part 2, we can use the fact that in the second half of the list,
    // each element is dependent only on the ones after it from the previous phase.
    std::vector<int> full_list;
    full_list.reserve(original_list.size() * REPEATS);
    for (auto i = 0; i < REPEATS; ++i) {
        full_list.insert(full_list.end(), std::begin(original_list),
                        std::end(original_list));
    }

    size_t offset = 0;
    for (auto i = 0; i < OFFSET_LENGTH; ++i) {
        offset = offset * 10 + original_list[i];
    }

    if (offset < full_list.size() / 2 || offset >= full_list.size() - OUTPUT_LENGTH) {
        std::stringstream error_message;
        error_message << "Cannot compute solution for offset " << offset;
        throw std::runtime_error(error_message.str());
    }

    for (auto phase = 0; phase < NUM_PHASES; ++phase) {
        for (size_t i = full_list.size() - 2; i >= offset; --i) {
            full_list[i] = std::abs(full_list[i] +  full_list[i + 1]) % 10;
        }
    }
    auto part2_answer = 0;
    for (auto i = offset; i < offset + OUTPUT_LENGTH; ++i) {
        part2_answer = part2_answer * 10 + full_list[i];
    }

    std::cout << "PART 1" << std::endl;
    std::cout << "First digits of final list: " << part1_answer << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "Digits at given offset: " << part2_answer << std::endl;
    return 0;
}
