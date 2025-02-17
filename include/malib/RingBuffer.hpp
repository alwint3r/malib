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

enum class OverwritePolicy {
  Discard,   // Discard new elements when buffer is full
  Overwrite  // Overwrite oldest elements when buffer is full
};

template <std::copyable T, size_t Capacity,
          OverwritePolicy Policy = OverwritePolicy::Discard>
class RingBuffer {
  static_assert(Capacity > 0);

 public:
  using value_type = T;

  RingBuffer() noexcept = default;
  ~RingBuffer() noexcept = default;
  RingBuffer(const RingBuffer&) = default;
  RingBuffer& operator=(const RingBuffer&) = default;
  RingBuffer(RingBuffer&&) noexcept = default;
  RingBuffer& operator=(RingBuffer&&) noexcept = default;

  /**
   * @brief Pushes a value into the ring buffer
   *
   * If the buffer is full, the behavior depends on the OverwritePolicy:
   * - For Discard policy: Returns BufferFull error without modifying the buffer
   * - For Overwrite policy: Overwrites the oldest value and updates head/tail
   * accordingly
   *
   * @param value The value to push into the buffer
   * @return Error::Ok on successful push, Error::BufferFull if buffer is full
   * and Discard policy is used
   * @thread_safety Thread-safe through internal mutex
   */
  Error push(const T& value) {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      if constexpr (Policy == OverwritePolicy::Discard) {
        return Error::BufferFull;
      } else {
        buffer_[head_] = value;
        increment_head();
        tail_ = head_;
        return Error::Ok;
      }
    }

    buffer_[tail_] = value;
    increment_tail();
    count_++;
    return Error::Ok;
  }

  /**
   * @brief Pushes a new element into the ring buffer using move semantics.
   *
   * @details Depending on the buffer's OverwritePolicy:
   *          - For Discard policy: Returns Error::BufferFull if buffer is full
   *          - For Overwrite policy: Overwrites oldest element if buffer is
   * full
   *
   * @param value The value to be moved into the buffer
   * @return Error::Ok if successful, Error::BufferFull if buffer is full (in
   * Discard policy)
   *
   * @thread_safety Thread-safe (protected by mutex)
   */
  Error push(T&& value) {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (count_ == Capacity) {
      if constexpr (Policy == OverwritePolicy::Discard) {
        return Error::BufferFull;
      } else {
        buffer_[head_] = std::move(value);
        increment_head();
        tail_ = head_;
        return Error::Ok;
      }
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

  [[nodiscard]] size_t size() const noexcept { return count_; }

  [[nodiscard]] bool empty() const noexcept { return count_ == 0; }

  [[nodiscard]] bool full() const noexcept { return count_ == Capacity; }

  [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }

  [[nodiscard]] size_t free_space() const noexcept { return Capacity - count_; }

  void clear() {
    std::scoped_lock<std::mutex> lock(mutex_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

  std::expected<std::size_t, Error> write(const T* data, std::size_t size) {
    std::scoped_lock<std::mutex> lock(mutex_);

    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if (size == 0) {
      return 0;
    }

    if constexpr (Policy == OverwritePolicy::Discard) {
      if (free_space() < size) {
        return std::unexpected(Error::BufferFull);
      }
    }

    const size_t available_space = free_space();
    const size_t write_size = Policy == OverwritePolicy::Discard
                                  ? std::min(size, available_space)
                                  : size;

    size_t elements_written = 0;
    while (elements_written < write_size) {
      const size_t space_to_end = Capacity - tail_;
      const size_t chunk_size =
          std::min(space_to_end, write_size - elements_written);

      std::copy_n(data + elements_written, chunk_size, buffer_.begin() + tail_);
      elements_written += chunk_size;

      if (Policy == OverwritePolicy::Overwrite &&
          count_ + chunk_size > Capacity) {
        const size_t overflow = count_ + chunk_size - Capacity;
        head_ = (tail_ + chunk_size) % Capacity;
        count_ = Capacity;
      } else {
        count_ += chunk_size;
      }

      tail_ = (tail_ + chunk_size) % Capacity;
    }

    return elements_written;
  }

  std::expected<std::size_t, Error> read(T* data, std::size_t size) {
    std::scoped_lock<std::mutex> lock(mutex_);

    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if (size == 0 || count_ == 0) {
      return 0;
    }

    const size_t read_size = std::min(size, count_);
    size_t elements_read = 0;

    while (elements_read < read_size) {
      const size_t data_to_end = Capacity - head_;
      const size_t chunk_size =
          std::min(data_to_end, read_size - elements_read);

      std::copy_n(buffer_.begin() + head_, chunk_size, data + elements_read);
      elements_read += chunk_size;
      count_ -= chunk_size;
      head_ = (head_ + chunk_size) % Capacity;
    }

    return elements_read;
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
