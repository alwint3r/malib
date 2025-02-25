#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "Error.hpp"

namespace malib {
/**
 * @brief A thread-safe queue implementation that allows waiting for elements
 * 
 * WaitableQueue provides a synchronized queue with blocking operations. It uses
 * mutex and condition variable to ensure thread safety and enable waiting for
 * elements when the queue is empty.
 * 
 * @tparam T The type of elements stored in the queue
 * 
 * Thread safety: This class is thread-safe and can be safely accessed from multiple threads
 */
template <typename T>
struct WaitableQueue {
  void push(T task) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(task);
    cv_.notify_one();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    auto item = queue_.front();
    queue_.pop();
    return item;
  }

 private:
  std::queue<T> queue_{};
  mutable std::mutex mutex_{};
  std::condition_variable cv_{};
};
};  // namespace malib