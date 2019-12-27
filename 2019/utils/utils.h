#include <chrono>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>


std::ifstream open_input_file(int argc, char **argv);


// A class that contains multiple queues,
// each protected by its own mutex.
template <typename T>
class MultiQueue {
public:
    void push_queue(size_t queue_num, const T &value) {
        auto& [mtx, que] = queues[queue_num];
        std::lock_guard<std::mutex> guard(mtx);
        que.push(value);
    }

    // Blocks if the queue is empty
    T pop_queue(size_t queue_num) {
        auto& [mtx, que] = queues[queue_num];
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        // Wait for an item to be added to the queue
        while (true) {
            lock.lock();
            if (!que.empty()) {
                // Don't unlock mutex here
                break;
            }
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // mutex is still locked
        auto value = que.front();
        que.pop();
        return value;
    }

private:
    std::map<size_t, std::pair<std::mutex, std::queue<T> > > queues;
};
