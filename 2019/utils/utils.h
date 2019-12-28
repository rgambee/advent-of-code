#include <chrono>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <deque>
#include <thread>
#include <utility>
#include <vector>


std::ifstream open_input_file(int argc, char **argv);


// A class that contains multiple queues,
// all protected by a single mutex.
template <typename T>
class MultiQueue {
public:
    void push_queue(size_t queue_num, const std::initializer_list<T> &values) {
        std::lock_guard<std::mutex> guard(top_level_mutex);
        auto& que = queues[queue_num];
        for (auto &val: values) {
            que.push_back(val);
        }
    }

    void push_queue(size_t queue_num, const T &value) {
        return push_queue(queue_num, {value});
    }

    // Return a vector of several items, blocking until all items are available
    std::vector<T> pop_queue_multiple_blocking(size_t queue_num, size_t num_values) {
        std::unique_lock<std::mutex> lock(top_level_mutex);
        auto& que = queues[queue_num];
        // Wait for items to be added to the queue
        while (true) {
            if (que.size() >= num_values) {
                // Don't unlock mutex here
                break;
            }
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
        }
        // mutex is still locked
        auto start = que.begin();
        auto stop = std::next(start, num_values);
        std::vector<T> values(start, stop);
        que.erase(start, stop);
        return values;
    }

    // Return a single item, blocking until it's available
    T pop_queue_blocking(size_t queue_num) {
        auto values = pop_queue_multiple_blocking(queue_num, 1);
        return values[0];
    }

    // Return default_value if the queue is empty
    T pop_queue_nonblocking(size_t queue_num, T default_value) {
        auto result = default_value;
        std::lock_guard<std::mutex> guard(top_level_mutex);
        auto& que = queues[queue_num];
        if (!que.empty()) {
            result = que.front();
            que.pop_front();
        }
        return result;
    }

    size_t get_queue_size(size_t queue_num) {
        std::lock_guard<std::mutex> guard(top_level_mutex);
        auto& que = queues[queue_num];
        return que.size();
    }

private:
    std::mutex top_level_mutex;
    std::map<size_t, std::deque<T> > queues;
};
