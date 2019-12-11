#include <iostream>
#include <sstream>

#include "intcode.h"
#include "utils.h"


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    std::cout << "PART 1" << std::endl;
    std::cout << "BOOST program output for input 1: ";
    auto program_input = std::stringstream{"1"};
    run_intcode_program(program, program_input, std::cout);
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    program_input = std::stringstream{"2"};
    run_intcode_program(program, program_input, std::cout);
    return 0;
}
