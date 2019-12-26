#include <iostream>
#include <string>

#include "intcode.h"
#include "utils.h"


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    std::string line;
    auto string_index = -1;
    auto intcode_input = [&line, &string_index]() -> intcode_type {
        if (string_index < 0) {
            std::getline(std::cin, line);
            ++string_index;
        }
        if (string_index < static_cast<int>(line.size())) {
            return static_cast<intcode_type>(line[string_index++]);
        }
        string_index = -1;
        return static_cast<intcode_type>('\n');
    };

    auto intcode_output = [](intcode_type output) -> void {
        std::cout << static_cast<char>(output);
    };

    run_intcode_program(program, intcode_input, intcode_output);
    return 0;
}
