#include <functional>
#include <istream>
#include <ostream>
#include <unordered_map>
#include <vector>


using intcode_type = long long;
using program_type = std::unordered_map<intcode_type, intcode_type>;

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

Opcode int_to_opcode(intcode_type integer);

enum class Mode {
    POSITIONAL = 0,
    IMMEDIATE = 1,
    RELATIVE = 2
};

std::vector<Mode> int_to_modes(intcode_type integer, int num_operands);


program_type load_intcode_program(std::istream &input_stream);


intcode_type run_intcode_program(program_type program,
                                 std::istream &input = std::cin,
                                 std::ostream &output = std::cout);

intcode_type run_intcode_program(program_type program,
                                 std::function<intcode_type()> input,
                                 std::function<void(intcode_type)> output);
