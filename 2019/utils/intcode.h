#include <functional>
#include <istream>
#include <ostream>
#include <vector>


enum class Opcode {
    ADD = 1,
    MULTIPLY = 2,
    INPUT = 3,
    OUTPUT = 4,
    JUMP_TRUE = 5,
    JUMP_FALSE = 6,
    LESS_THAN = 7,
    EQUALS = 8,
    REL_BASE = 9,
    END = 99
};

Opcode int_to_opcode(int integer);

enum class Mode {
    POSITIONAL = 0,
    IMMEDIATE = 1,
    RELATIVE = 2
};

std::vector<Mode> int_to_modes(int integer, int num_operands);


std::vector<int> load_intcode_program(std::istream &input_stream);


int run_intcode_program(std::vector<int> numbers,
                        std::istream &input = std::cin,
                        std::ostream &output = std::cout);

int run_intcode_program(std::vector<int> numbers,
                        std::function<int()> input,
                        std::function<void(int)> output);
