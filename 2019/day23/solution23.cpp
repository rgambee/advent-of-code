#include <iostream>
#include <map>
#include <thread>
#include <utility>

#include "intcode.h"
#include "utils.h"


constexpr size_t NUM_COMPUTERS = 50;
constexpr size_t NAT_ADDRESS = 255;
using packet_type = std::pair<intcode_type, intcode_type>;


class NAT {
public:
    void intercept_packet(MultiQueue<intcode_type> &mq) {
        auto packet = mq.pop_queue_multiple_blocking(NAT_ADDRESS, 2);
        last_x = packet[0];
        last_y = packet[1];
    }

    void release_packet(MultiQueue<intcode_type> &mq) {
        mq.push_queue(0, {last_x, last_y});
    }

    intcode_type last_x = -1, last_y = -1;
};


std::thread launch_thread(const program_type &program,
                          MultiQueue<intcode_type> &output_queue,
                          MultiQueue<intcode_type> &input_queue,
                          size_t address) {
    auto intcode_input = [&input_queue, address]() -> intcode_type {
        return input_queue.pop_queue_nonblocking(address, -1);
    };

    auto intcode_output = [&output_queue, &input_queue, address](intcode_type output) -> void {
        output_queue.push_queue(address, output);
        if (output_queue.get_queue_size(address) >= 3) {
            // This computer has accumulated an address, an X and a Y
            // in its output queue. Pass the packet to the destination.
            auto values = output_queue.pop_queue_multiple_blocking(address, 3);
            auto dest = values[0], x = values[1], y = values[2];
            if (dest < 0 || (dest >= static_cast<intcode_type>(NUM_COMPUTERS)
                             && dest != NAT_ADDRESS)) {
                std::cerr << "Invalid destination address: " << dest << std::endl;
                exit(5);
            }
            input_queue.push_queue(dest, {x, y});
        }
    };

    return std::thread([program, intcode_input, intcode_output]() -> void {
        run_intcode_program(program, intcode_input, intcode_output);
    });
}


int main(int argc, char **argv) {
    auto input_stream = open_input_file(argc, argv);
    auto program = load_intcode_program(input_stream);

    MultiQueue<intcode_type> output_queue, input_queue;
    // Initialize input queue with each computer's address
    for (size_t addr = 0; addr < NUM_COMPUTERS; ++addr) {
        input_queue.push_queue(addr, addr);
    }

    std::vector<std::thread> threads;
    for (size_t addr = 0; addr < NUM_COMPUTERS; ++addr) {
        threads.push_back(launch_thread(
            program, output_queue, input_queue, addr));
    }

    NAT nat_comp;
    intcode_type first_y_to_nat = -1;
    intcode_type previous_y_emitted = -1;
    while (true) {
        // Process any packets sent to NAT
        while (input_queue.get_queue_size(NAT_ADDRESS) >= 2) {
            nat_comp.intercept_packet(input_queue);
            if (first_y_to_nat == -1) {
                first_y_to_nat = nat_comp.last_y;
            }
        }
        // Check if network is idle
        // Hack: pass through the input queues multiple times to make sure
        // the network really is idle. What's a better way to do this?
        bool idle = true;
        for (auto repeat = 0; repeat < 5; ++repeat) {
            for (size_t addr = 0; addr < NUM_COMPUTERS; ++addr) {
                if (input_queue.get_queue_size(addr) > 0) {
                    idle = false;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (idle) {
            std::cout << "Emiting packet (" << nat_comp.last_x;
            std::cout << ", " << nat_comp.last_y << ") to address 0" << std::endl;
            if (previous_y_emitted == nat_comp.last_y) {
                break;
            }
            previous_y_emitted = nat_comp.last_y;
            nat_comp.release_packet(input_queue);
        }
    }

    // FIXME: clean up threads more cleanly

    std::cout << "PART 1" << std::endl;
    std::cout << "Y value of first packet to " << NAT_ADDRESS;
    std::cout << ": " << first_y_to_nat << std::endl;
    std::cout << std::endl;
    std::cout << "PART 2" << std::endl;
    std::cout << "First repeated Y value: " << previous_y_emitted << std::endl;
    return 0;
}
