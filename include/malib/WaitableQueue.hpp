#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "Error.hpp"

namespace malib {
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
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
};
};  // namespace malib