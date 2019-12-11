#include <cmath>
#include <iostream>
#include <string>

#include "intcode.h"


Opcode int_to_opcode(intcode_type integer) {
    // Could simply cast to an Opcode,
    // but that wouldn't reject unknown inputs.
    switch(integer % 100) {
        case 1:
            return Opcode::ADD;
        case 2:
            return Opcode::MULTIPLY;
        case 3:
            return Opcode::INPUT;
        case 4:
            return Opcode::OUTPUT;
        case 5:
            return Opcode::JUMP_TRUE;
        case 6:
            return Opcode::JUMP_FALSE;
        case 7:
            return Opcode::LESS_THAN;
        case 8:
            return Opcode::EQUALS;
        case 9:
            return Opcode::REL_BASE;
        case 99:
            return Opcode::END;
        default:
            std::cerr << "Unknown opcode: " << integer << std::endl;
            exit(3);
    }
}


std::vector<Mode> int_to_modes(intcode_type integer, int num_operands) {
    std::vector<Mode> result(num_operands, Mode::POSITIONAL);
    integer /= 100;     // Remove opcode (trailing two digits)
    for (auto place = 0; place < num_operands; place++) {
        intcode_type digit = (integer / static_cast<intcode_type>(
            std::round(std::pow(10, place)))) % 10;
        switch(digit) {
            case 0:
                result[place] = Mode::POSITIONAL;
                break;
            case 1:
                result[place] = Mode::IMMEDIATE;
                break;
            case 2:
                result[place] = Mode::RELATIVE;
                break;
            default:
                std::cerr << "Unknown mode: " << digit;
                exit(3);
        }
    }
    return result;
}


program_type load_intcode_program(std::istream &input_stream) {
    program_type program;
    std::string num_str;
    intcode_type address = 0;
    while (std::getline(input_stream, num_str, ',')) {
        program[address] = std::stoll(num_str);
        ++address;
    }
    return program;
}


void check_index(intcode_type index) {
    if (index < 0) {
        std::cerr << "Index out of range: " << index << std::endl;
        exit(4);
    }
}


intcode_type run_intcode_program(program_type program,
                                 std::istream &input,
                                 std::ostream &output) {
    return run_intcode_program(
        program,
        [&input]() -> intcode_type {intcode_type val = -1; input >> val; return val;},
        [&output](intcode_type val) -> void {output << val;});
}


intcode_type run_intcode_program(program_type program,
                                 std::function<intcode_type()> input,
                                 std::function<void(intcode_type)> output) {
    auto pc = 0, relative_base = 0;
    bool done = false;
    while (!done) {
        auto opcode = int_to_opcode(program[pc]);
        switch (opcode) {
            case Opcode::END:
                done = true;
                break;
            case Opcode::INPUT:
            case Opcode::OUTPUT:
            case Opcode::REL_BASE: {
                int num_operands = 1;
                auto modes = int_to_modes(program[pc], num_operands);
                if (opcode == Opcode::INPUT && modes[0] != Mode::POSITIONAL
                    && modes[0] != Mode::RELATIVE) {
                    std::cerr << "Opcode " << static_cast<int>(opcode);
                    std::cerr << " expects positional operand mode" << std::endl;
                    exit(3);
                }
                auto parameter = program[pc+1];
                intcode_type value;
                switch (modes[0]) {
                    case Mode::POSITIONAL:
                        check_index(parameter);
                        value = program[parameter];
                        break;
                    case Mode::IMMEDIATE:
                        value = parameter;
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + parameter);
                        value = program[relative_base + parameter];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                        exit(3);
                }
                switch (opcode) {
                    case Opcode::INPUT:
                        switch (modes[0]) {
                            case Mode::POSITIONAL:
                                program[parameter] = input();
                                break;
                            case Mode::RELATIVE:
                                program[relative_base + parameter] = input();
                                break;
                            default:
                                std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                                exit(3);
                        }
                    case Opcode::OUTPUT:
                        output(value);
                        break;
                    case Opcode::REL_BASE:
                        relative_base += value;
                        break;
                    default:
                        std::cerr << "Unexpected opcode: " << static_cast<int>(opcode) << std::endl;
                        exit(3);
                }
                pc += num_operands + 1;
                break;
            }
            case Opcode::JUMP_TRUE:
            case Opcode::JUMP_FALSE: {
                int num_operands = 2;
                auto modes = int_to_modes(program[pc], num_operands);
                bool condition = false;
                intcode_type destination = -1;
                switch (modes[0]) {
                    case Mode::POSITIONAL:
                        check_index(program[pc+1]);
                        condition = static_cast<bool>(program[program[pc+1]]);
                        break;
                    case Mode::IMMEDIATE:
                        condition = static_cast<bool>(program[pc+1]);
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + program[pc+1]);
                        condition = static_cast<bool>(program[relative_base + program[pc+1]]);
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                        exit(3);
                }
                switch (modes[1]) {
                    case Mode::POSITIONAL:
                        check_index(program[pc+2]);
                        destination = program[program[pc+2]];
                        break;
                    case Mode::IMMEDIATE:
                        destination = program[pc+2];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + program[pc+2]);
                        destination = program[relative_base + program[pc+2]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[1]) << std::endl;
                        exit(3);
                }
                switch (opcode) {
                    case Opcode::JUMP_TRUE:
                        pc = condition ? destination : pc + num_operands + 1;
                        break;
                    case Opcode::JUMP_FALSE:
                        pc = !condition ? destination : pc + num_operands + 1;
                        break;
                    default:
                        std::cerr << "Unexpected opcode: " << static_cast<int>(opcode) << std::endl;
                        exit(3);
                }
                break;
            }
            case Opcode::ADD:
            case Opcode::MULTIPLY:
            case Opcode::LESS_THAN:
            case Opcode::EQUALS: {
                int num_operands = 3;
                auto modes = int_to_modes(program[pc], num_operands);
                intcode_type input_a = -1, input_b = -1, output_index;
                switch (modes[0]) {
                    case Mode::POSITIONAL:
                        check_index(program[pc+1]);
                        input_a = program[program[pc+1]];
                        break;
                    case Mode::IMMEDIATE:
                        input_a = program[pc+1];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + program[pc+1]);
                        input_a = program[relative_base + program[pc+1]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                        exit(3);
                }
                switch (modes[1]) {
                    case Mode::POSITIONAL:
                        check_index(program[pc+2]);
                        input_b = program[program[pc+2]];
                        break;
                    case Mode::IMMEDIATE:
                        input_b = program[pc+2];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + program[pc+2]);
                        input_b = program[relative_base + program[pc+2]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[1]) << std::endl;
                        exit(3);
                }
                switch (modes[2]) {
                    case Mode::POSITIONAL:
                        output_index = program[pc+3];
                        check_index(output_index);
                        break;
                    case Mode::RELATIVE:
                        output_index = relative_base + program[pc+3];
                        check_index(output_index);
                        break;
                    default:
                        std::cerr << "Opcode " << static_cast<int>(opcode);
                        std::cerr << " expects positional or relative mode";
                        std::cerr << " for final operand" << std::endl;
                        exit(3);
                }
                intcode_type result = -1;
                switch (opcode) {
                    case Opcode::ADD:
                        result = input_a + input_b;
                        break;
                    case Opcode::MULTIPLY:
                        result = input_a * input_b;
                        break;
                    case Opcode::LESS_THAN:
                        result = input_a < input_b ? 1 : 0;
                        break;
                    case Opcode::EQUALS:
                        result = input_a == input_b ? 1 : 0;
                        break;
                    default:
                        std::cerr << "Unexpected opcode: " << static_cast<int>(opcode) << std::endl;
                        exit(3);
                }
                program[output_index] = result;
                pc += num_operands + 1;
                break;
            }
            default:
                std::cerr << "Unexpected opcode: " << static_cast<int>(opcode) << std::endl;
                exit(3);
        }
    }
    return program[0];
}
