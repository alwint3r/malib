#pragma once

#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <mutex>
#include <vector>

#include "malib/Error.hpp"

namespace malib {
template <std::copyable T, size_t Capacity>
class RingBuffer {
 public:
  using ValueType = T;

  RingBuffer() = default;
  ~RingBuffer() = default;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  Error push(const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      return Error::BufferFull;
    }

    buffer_[tail_] = value;
    tail_ = (tail_ + 1) % Capacity;
    count_++;
    return Error::Ok;
  }

  std::expected<T, Error> pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return std::unexpected(Error::BufferEmpty);
    }

    T value = buffer_[head_];
    head_ = (head_ + 1) % Capacity;
    count_--;
    return value;
  }

  std::expected<T, Error> peek() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return std::unexpected(Error::BufferEmpty);
    }
    return buffer_[head_];
  }

  std::pair<std::array<T, Capacity>, size_t> consume_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::array<T, Capacity> elements;
    size_t count = 0;
    while (count_ > 0) {
      elements[count++] = buffer_[head_];
      head_ = (head_ + 1) % Capacity;
      count_--;
    }
    return {elements, count};
  }

  size_t size() const { return count_; }

  bool empty() const { return count_ == 0; }

  bool full() const { return count_ == Capacity; }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

 private:
  size_t head_{0};
  size_t tail_{0};
  size_t count_{0};
  std::array<T, Capacity> buffer_;
  mutable std::mutex mutex_;
};
}  // namespace malib
