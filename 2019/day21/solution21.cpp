#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "intcode.h"
#include "utils.h"


enum class Register {
    TEMP = 'T',
    JUMP = 'J',
    ONE = 'A',
    TWO = 'B',
    THREE = 'C',
    FOUR = 'D',
    FIVE = 'E',
    SIX = 'F',
    SEVEN = 'G',
    EIGHT = 'H',
    NINE = 'I'
};

enum class Operation {
    AND,
    OR,
    NOT
};


class Instruction {
public:
    Instruction(Operation op, Register reg1, Register reg2):
        operation(op), register1(reg1), register2(reg2) {}

    void append_to_stream(std::stringstream &stream) const {
        std::string op_str;
        switch (operation) {
            case Operation::AND:
                op_str = "AND";
                break;
            case Operation::OR:
                op_str = "OR";
                break;
            case Operation::NOT:
                op_str = "NOT";
                break;
        }
        stream << op_str << " " << static_cast<char>(register1) << " ";
        stream << static_cast<char>(register2) << '\n';
    }

    Operation operation = Operation::NOT;
    Register register1 = Register::TEMP;
    Register register2 = Register::TEMP;
};


std::stringstream instructions_to_stream(
        const std::vector<Instruction> &instructions, bool part1 = true) {
    std::stringstream stream;
    for (auto &instr: instructions) {
        instr.append_to_stream(stream);
    }
    if (part1) {
        stream << "WALK\n";
    } else {
        stream << "RUN\n";
    }
    return stream; 
}


std::function<void(intcode_type)> print_intcode_output() {
    auto intcode_output = [](intcode_type output) -> void {
        if (output > std::numeric_limits<signed char>::max()) {
            std::cout << output << std::endl;
        } else {
            std::cout << static_cast<char>(output);
        }
    };
    return intcode_output;
}


void run_springdroid_program(const std::vector<Instruction> &instructions,
                             const program_type &program,
                             bool part1 = true) {
    auto instructions_string = instructions_to_stream(
        instructions, part1).str();
    size_t string_index = 0;
    auto intcode_input = [&]() -> intcode_type {
        auto result = instructions_string[string_index++];
        return static_cast<intcode_type>(result);
    };
    run_intcode_program(program, intcode_input, print_intcode_output());
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    std::vector<Instruction> instructions_part1 {
        // Jump if hole three steps away
        Instruction(Operation::NOT, Register::THREE, Register::TEMP),
        Instruction(Operation::OR, Register::TEMP, Register::JUMP),
        // Jump if directly in front of hole
        Instruction(Operation::NOT, Register::ONE, Register::TEMP),
        Instruction(Operation::OR, Register::TEMP, Register::JUMP),
        // Don't jump if destination is hole
        Instruction(Operation::AND, Register::FOUR, Register::JUMP)
    };

    std::cout << "PART 1" << std::endl;
    run_springdroid_program(instructions_part1, program, true);

    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    return 0;
}
