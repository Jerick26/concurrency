#include "thread_pool_waitable_tasks.h"

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
  usigned long const length = std::distance(first, last);
  if (length <= 0)
    return init;
  unsigned long const block_size = 25;
  unsigned long const num_blocks = (length+block_size-1)/block_size;

  std::vector<std::future<T> > futures(num_blocks-1);
  thread_pool pool;

  Iterator block_start = first;
  for (unsigned long i=0; i<(num_blocks-1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    futures[i] = pool.submit(accumulate_block<Iterator, T>());
    block_start = block_end;
  }
  T last_result = accumulate_block<Iterator, T>()(block_start, last);
  T result = init;
  for (unsigned long i=0; i<(num_block-1); ++i) {
    result += futures[i].get();
  }
  result += last_result;
  return result;
};
