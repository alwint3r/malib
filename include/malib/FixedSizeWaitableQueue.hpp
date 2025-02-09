#pragma once

#include <condition_variable>
#include <mutex>

#include "Error.hpp"
#include "RingBuffer.hpp"

namespace malib {
template <typename T, size_t N>
class FixedSizeWaitableQueue {
 public:
  Error push(T task) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (buffer_.full()) {
      return Error::QueueFull;
    }
    auto result = buffer_.push(std::move(task));
    if (result == Error::Ok) {
      cv_.notify_one();
    }
    return result;
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !buffer_.empty(); });
    auto result = buffer_.pop();
    return *result;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.empty();
  }

  bool full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.full();
  }

 private:
  RingBuffer<T, N> buffer_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
};
}  // namespace malib
