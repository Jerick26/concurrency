#include <utility>
#include <vector>
#include <type_traits>
#include <future>
#include <thread>

class thread_pool {
  std::atomic_bool done;
  threadsafe_queue<function_wrapper> work_queue;
  std::vector<std::thread> threads;
  join_threads joiner;

  void worker_thread() {
    while (!done) {
      function_wrapper task;
      if (work_queue.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }
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
  std::future<typename std::result_of<FunctionType()>::type>
  submit(FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());
    work_queue.push(std::move(task));
    return res;
  }

  void run_pending_task() {
    function_wrapper task;
    if (work_queue.try_pop(task)) {
      task();
    } else {
      std::this_thread::yield();
    }
  }
};
