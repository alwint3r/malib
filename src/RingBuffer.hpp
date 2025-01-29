#pragma once

#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <mutex>

namespace malib {
template <std::copyable T, size_t Capacity>
class RingBuffer {
 public:
  enum class Error { OK, Full, Empty };
  RingBuffer() = default;
  ~RingBuffer() = default;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  [[nodiscard]]
  Error push(const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      return Error::Full;
    }

    buffer_[tail_] = value;
    tail_ = (tail_ + 1) % Capacity;
    count_++;
    return Error::OK;
  }

  std::expected<T, Error> pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return std::unexpected(Error::Empty);
    }

    T value = buffer_[head_];
    head_ = (head_ + 1) % Capacity;
    count_--;
    return value;
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
  std::mutex mutex_;
};
}  // namespace malib
