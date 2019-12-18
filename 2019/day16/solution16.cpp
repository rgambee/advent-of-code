#include <iostream>
#include <valarray>
#include <cmath>
#include <string>

#include "utils.h"


constexpr auto NUM_PHASES = 100;
const std::valarray<int> BASE_PATTERN{0, 1, 0, -1};


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    std::string line;
    std::getline(input_stream, line);
    std::valarray<int> list(line.size());
    for (std::string::size_type i = 0; i < line.size(); ++i) {
        list[i] = line[i] - '0';
    }

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

    std::cout << "PART 1" << std::endl;
    std::cout << "First 8 digits of final list: ";
    for (int i = 0; i < 8; ++i) {
        std::cout << list[i];
    }
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
