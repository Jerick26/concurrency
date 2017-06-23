#include <thread>
#include <atomic>
#include "threadsafe_queue.h"

class thread_pool {
public:
  thread_pool() :
      done(false), joiner(threads) {
    unsigned int const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i=0; i<thread_count; ++i) {
        threads.push_back(std::thread(&thread_pool::work_thread, this));
      }
    } catch (...) {
      done = true;
      throw;
    }
  }
  ~thread_pool() {
    done = true;
  }
  template<typename FunctionType>
  void submit(FunctionType f) {
    work_queue.push(std::function<void()>(f));
  }
private:
  void work_thread() {
    while (!done) {
      std::function<void()> task;
      if (work_queue.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }
private:
  std::atomic_bool done;
  thread_safe_queue<std::function<void()> > work_queue;
  std::vector<std::thread> threads;
  join_threads joiner;
};
