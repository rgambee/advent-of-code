#include <iostream>
#include <sstream>
#include <stdexcept>

#include "utils.h"


std::ifstream open_input_file(int argc, char **argv) {
    if (argc != 2) {
        std::stringstream error_message;
        error_message << "Usage: " << argv[0] << " input.txt";
        throw std::runtime_error(error_message.str());
    }

    std::ifstream input_stream(argv[1]);
    if (!input_stream) {
        std::stringstream error_message;
        error_message << "File not found: " << argv[1];
        throw std::runtime_error(error_message.str());
    }

    return input_stream;
}
