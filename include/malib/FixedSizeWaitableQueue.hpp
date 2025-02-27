#pragma once

#include <condition_variable>
#include <expected>
#include <mutex>

#include "Error.hpp"
#include "RingBuffer.hpp"

namespace malib {
/**
 * @brief A thread-safe fixed-size queue with blocking and non-blocking operations
 * 
 * This class implements a fixed-size queue that can be safely accessed from multiple threads.
 * It provides both blocking and non-blocking operations for pushing and popping elements.
 * 
 * @tparam T The type of elements stored in the queue
 * @tparam N The fixed size of the queue
 * 
 * Thread safety: Thread safe. All public methods are protected by internal mutex
 * 
 * @note This implementation uses a condition variable for blocking operations
 * @example
 */
template <typename T, size_t N>
class FixedSizeWaitableQueue {
 public:
  Error push(T&& item) {
    return push_impl(std::forward<T>(item));
  }

  Error push(const T& item) {
    return push_impl(item);
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !buffer_.empty(); });
    auto result = buffer_.pop();
    return *result;
  }

  std::expected<T, Error> try_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (buffer_.empty()) {
      return std::unexpected(Error::BufferEmpty);
    }
    return buffer_.pop();
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
  template <typename U>
  Error push_impl(U&& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (buffer_.full()) {
      return Error::QueueFull;
    }
    auto result = buffer_.push(std::forward<U>(item));
    if (result == Error::Ok) {
      cv_.notify_one();
    }
    return result;
  }

  RingBuffer<T, N> buffer_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
};
}  // namespace malib
