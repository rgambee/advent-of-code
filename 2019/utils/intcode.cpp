#include <cmath>
#include <iostream>
#include <string>

#include "intcode.h"


Opcode int_to_opcode(int integer) {
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


std::vector<Mode> int_to_modes(int integer, int num_operands) {
    std::vector<Mode> result(num_operands, Mode::POSITIONAL);
    integer /= 100;     // Remove opcode (trailing two digits)
    for (auto place = 0; place < num_operands; place++) {
        int digit = (integer / static_cast<int>(std::round(std::pow(10, place)))) % 10;
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
    program_type numbers;
    std::string num_str;
    int address = 0;
    while (std::getline(input_stream, num_str, ',')) {
        numbers[address] = std::stoi(num_str);
        ++address;
    }
    return numbers;
}


void check_index(int index) {
    if (index < 0) {
        std::cerr << "Index out of range: " << index << std::endl;
        exit(4);
    }
}


intcode_type run_intcode_program(program_type numbers,
                                 std::istream &input,
                                 std::ostream &output) {
    return run_intcode_program(
        numbers,
        [&input]() -> int {intcode_type val = -1; input >> val; return val;},
        [&output](intcode_type val) -> void {output << val;});
}


int run_intcode_program(program_type numbers,
                        std::function<int()> input,
                        std::function<void(int)> output) {
    auto pc = 0, relative_base = 0;
    bool done = false;
    while (!done) {
        auto opcode = int_to_opcode(numbers[pc]);
        switch (opcode) {
            case Opcode::END:
                done = true;
                break;
            case Opcode::INPUT:
            case Opcode::OUTPUT:
            case Opcode::REL_BASE: {
                int num_operands = 1;
                auto modes = int_to_modes(numbers[pc], num_operands);
                if (opcode == Opcode::INPUT && modes[0] != Mode::POSITIONAL) {
                    std::cerr << "Opcode " << static_cast<int>(opcode);
                    std::cerr << " expects positional operand mode" << std::endl;
                    exit(3);
                }
                if (opcode == Opcode::REL_BASE && modes[0] != Mode::IMMEDIATE) {
                    std::cerr << "Opcode " << static_cast<int>(opcode);
                    std::cerr << " expects immediate operand mode" << std::endl;
                    exit(3);
                }
                auto parameter = numbers[pc+1];
                if (modes[0] == Mode::POSITIONAL) {
                    check_index(parameter);
                } else if (modes[0] == Mode::RELATIVE) {
                    check_index(relative_base + parameter);
                }
                switch (opcode) {
                    case Opcode::INPUT:
                        numbers[parameter] = input();
                        break;
                    case Opcode::OUTPUT:
                        switch (modes[0]) {
                            case Mode::POSITIONAL:
                                output(numbers[parameter]);
                                break;
                            case Mode::IMMEDIATE:
                                output(parameter);
                                break;
                            case Mode::RELATIVE:
                                output(numbers[relative_base + parameter]);
                                break;
                            default:
                                std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                                exit(3);
                        }
                        break;
                    case Opcode::REL_BASE:
                        relative_base += parameter;
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
                auto modes = int_to_modes(numbers[pc], num_operands);
                int condition = -1, destination = -1;
                switch (modes[0]) {
                    case Mode::POSITIONAL:
                        check_index(numbers[pc+1]);
                        condition = numbers[numbers[pc+1]];
                        break;
                    case Mode::IMMEDIATE:
                        condition = numbers[pc+1];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + numbers[pc+1]);
                        condition = numbers[relative_base + numbers[pc+1]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                        exit(3);
                }
                switch (modes[1]) {
                    case Mode::POSITIONAL:
                        check_index(numbers[pc+2]);
                        destination = numbers[numbers[pc+2]];
                        break;
                    case Mode::IMMEDIATE:
                        destination = numbers[pc+2];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + numbers[pc+2]);
                        destination = numbers[relative_base + numbers[pc+2]];
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
                auto modes = int_to_modes(numbers[pc], num_operands);
                int input_a = -1, input_b = -1, output_index;
                switch (modes[0]) {
                    case Mode::POSITIONAL:
                        check_index(numbers[pc+1]);
                        input_a = numbers[numbers[pc+1]];
                        break;
                    case Mode::IMMEDIATE:
                        input_a = numbers[pc+1];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + numbers[pc+1]);
                        input_a = numbers[relative_base + numbers[pc+1]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[0]) << std::endl;
                        exit(3);
                }
                switch (modes[1]) {
                    case Mode::POSITIONAL:
                        check_index(numbers[pc+2]);
                        input_b = numbers[numbers[pc+2]];
                        break;
                    case Mode::IMMEDIATE:
                        input_b = numbers[pc+2];
                        break;
                    case Mode::RELATIVE:
                        check_index(relative_base + numbers[pc+2]);
                        input_b = numbers[relative_base + numbers[pc+2]];
                        break;
                    default:
                        std::cerr << "Unexpected mode: " << static_cast<int>(modes[1]) << std::endl;
                        exit(3);
                }
                switch (modes[2]) {
                    case Mode::POSITIONAL:
                        output_index = numbers[pc+3];
                        check_index(output_index);
                        break;
                    case Mode::RELATIVE:
                        output_index = relative_base + numbers[pc+3];
                        check_index(output_index);
                        break;
                    default:
                        std::cerr << "Opcode " << static_cast<int>(opcode);
                        std::cerr << " expects positional or relative mode";
                        std::cerr << " for final operand" << std::endl;
                        exit(3);
                }
                int result = -1;
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
                numbers[output_index] = result;
                pc += num_operands + 1;
                break;
            }
            default:
                std::cerr << "Unexpected opcode: " << static_cast<int>(opcode) << std::endl;
                exit(3);
        }
    }
    return numbers[0];
}
