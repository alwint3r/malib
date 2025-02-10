#pragma once

#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <mutex>
#include <vector>

#include "malib/Error.hpp"
#include "malib/concepts.hpp"

namespace malib {
template <std::copyable T, size_t Capacity>
class RingBuffer {
  static_assert(Capacity > 0);

 public:
  using value_type = T;

  RingBuffer() noexcept = default;
  ~RingBuffer() noexcept = default;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;
  RingBuffer(RingBuffer&&) noexcept = default;
  RingBuffer& operator=(RingBuffer&&) noexcept = default;

  /**
   * @brief Pushes a value into the ring buffer.
   *
   * This function attempts to push a value into the ring buffer. If the buffer
   * is full, it returns an error indicating that the buffer is full. Otherwise,
   * it inserts the value at the current tail position, increments the tail
   * position, and increases the count of elements in the buffer.
   *
   * @param value The value to be pushed into the buffer.
   * @return Error::Ok if the value was successfully pushed into the buffer,
   *         Error::BufferFull if the buffer is full.
   */
  Error push(const T& value) {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      return Error::BufferFull;
    }

    buffer_[tail_] = value;
    increment_tail();
    count_++;
    return Error::Ok;
  }

  /**
   * @brief Pushes a value into the ring buffer.
   *
   * This function attempts to push a value into the ring buffer. If the buffer
   * is full, it returns an error indicating that the buffer is full. Otherwise,
   * it inserts the value at the current tail position, increments the tail
   * position, and increases the count of elements in the buffer.
   *
   * @param value The value to be pushed into the buffer.
   * @return Error::Ok if the value was successfully pushed into the buffer,
   *         Error::BufferFull if the buffer is full.
   */
  Error push(T&& value) {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      return Error::BufferFull;
    }

    buffer_[tail_] = std::move(value);
    increment_tail();
    count_++;
    return Error::Ok;
  }

  /**
   * @brief Pops a value from the ring buffer.
   *
   * This function attempts to pop a value from the ring buffer. If the buffer
   * is empty, it returns an error indicating that the buffer is empty.
   * Otherwise, it retrieves the value at the current head position, increments
   * the head position, and decreases the count of elements in the buffer.
   *
   * @return A pair containing the popped value and an error code.
   */
  std::expected<T, Error> pop() {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return std::unexpected(Error::BufferEmpty);
    }

    T value = std::move(buffer_[head_]);
    increment_head();
    count_--;
    return value;
  }

  /**
   * @brief Peeks at the value at the head of the ring buffer.
   *
   * This function retrieves the value at the head of the ring buffer without
   * removing it. If the buffer is empty, it returns an error indicating that
   * the buffer is empty. Otherwise, it returns the value at the head of the
   * buffer.
   *
   * @return A pair containing the value at the head of the buffer and an error
   * code.
   */
  std::expected<T, Error> peek() const {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return std::unexpected(Error::BufferEmpty);
    }
    return buffer_[head_];
  }

  std::pair<std::array<T, Capacity>, size_t> consume_all() {
    std::scoped_lock<std::mutex> lock(mutex_);
    std::array<T, Capacity> elements;
    size_t count = 0;
    while (count_ > 0) {
      elements[count++] = std::move(buffer_[head_]);
      increment_head();
      count_--;
    }
    return {std::move(elements), count};
  }

  size_t size() const noexcept { return count_; }

  bool empty() const noexcept { return count_ == 0; }

  bool full() const noexcept { return count_ == Capacity; }

  constexpr size_t capacity() const noexcept { return Capacity; }

  void clear() {
    std::scoped_lock<std::mutex> lock(mutex_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

 private:
  /**
   * @brief Increments the head index of the ring buffer.
   *
   * This function increments the head index by one and wraps it around
   * using the modulo operation with the buffer's capacity. It ensures
   * that the head index stays within the valid range [0, Capacity-1].
   */
  inline void increment_head() noexcept { head_ = (head_ + 1) % Capacity; }

  /**
   * @brief Increments the tail index of the ring buffer.
   *
   * This function increments the tail index by one and wraps it around
   * using the modulo operation with the buffer's capacity. This ensures
   * that the tail index stays within the valid range of the buffer.
   */
  inline void increment_tail() noexcept { tail_ = (tail_ + 1) % Capacity; }

 private:
  size_t head_{0};
  size_t tail_{0};
  size_t count_{0};
  std::array<T, Capacity> buffer_{};
  mutable std::mutex mutex_{};
};

static_assert(std::same_as<RingBuffer<int, 10>::value_type, int>);
static_assert(container_like<RingBuffer<int, 10>>);
static_assert(poppable_container<RingBuffer<int, 10>>);
}  // namespace malib
