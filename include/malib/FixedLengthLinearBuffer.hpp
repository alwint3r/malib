#pragma once

#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <mutex>
#include <type_traits>

#include "malib/Error.hpp"

namespace malib {

template <typename T, size_t Capacity, bool ThreadSafe = false>
  requires std::copyable<T>
class FixedLengthLinearBuffer {
  static_assert(Capacity > 0);
  using value_type = T;
  using mutex_type = std::mutex;
  using lock_guard = std::lock_guard<mutex_type>;

 public:
  FixedLengthLinearBuffer() noexcept = default;
  ~FixedLengthLinearBuffer() noexcept = default;
  FixedLengthLinearBuffer(const FixedLengthLinearBuffer&) = default;
  FixedLengthLinearBuffer& operator=(const FixedLengthLinearBuffer&) = default;
  FixedLengthLinearBuffer(FixedLengthLinearBuffer&&) noexcept = default;
  FixedLengthLinearBuffer& operator=(FixedLengthLinearBuffer&&) noexcept =
      default;

  std::expected<std::size_t, Error> write(const T* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return write_impl(data, size);
    } else {
      return write_impl(data, size);
    }
  }

  template <typename U = T>
    requires(!std::is_trivially_copyable_v<U>)
  std::expected<std::size_t, Error> write_move(U* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return write_move_impl(data, size);
    } else {
      return write_move_impl(data, size);
    }
  }

  std::expected<std::size_t, Error> read(T* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return read_impl(data, size);
    } else {
      return read_impl(data, size);
    }
  }

  [[nodiscard]] size_t size() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return current_size_;
    } else {
      return current_size_;
    }
  }

  [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }

  [[nodiscard]] size_t free_space() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return Capacity - current_size_;
    } else {
      return Capacity - current_size_;
    }
  }

  [[nodiscard]] bool empty() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return current_size_ == 0;
    } else {
      return current_size_ == 0;
    }
  }

  [[nodiscard]] bool full() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return current_size_ == Capacity;
    } else {
      return current_size_ == Capacity;
    }
  }

  void clear() noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      clear_impl();
    } else {
      clear_impl();
    }
  }

  const T* data() const noexcept { return buffer_.data(); }
  T* data() noexcept { return buffer_.data(); }

  // Iterator types
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Iterator methods
  iterator begin() noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return buffer_.data();
    }
    return buffer_.data();
  }

  const_iterator begin() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return buffer_.data();
    }
    return buffer_.data();
  }

  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return buffer_.data() + current_size_;
    }
    return buffer_.data() + current_size_;
  }

  const_iterator end() const noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return buffer_.data() + current_size_;
    }
    return buffer_.data() + current_size_;
  }

  const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const noexcept { return rend(); }

 private:
  std::expected<std::size_t, Error> write_impl(const T* data,
                                               std::size_t size) {
    if (full()) {
      return std::unexpected(Error::BufferFull);
    }

    const auto write_size = std::min(Capacity - current_size_, size);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memcpy(buffer_.data() + current_size_, data, write_size * sizeof(T));
    } else {
      for (size_t i = 0; i < write_size; ++i) {
        buffer_[current_size_ + i] = data[i];
      }
    }

    current_size_ += write_size;
    return write_size;
  }

  template <typename U>
  std::expected<std::size_t, Error> write_move_impl(U* data, std::size_t size) {
    if (full()) {
      return std::unexpected(Error::BufferFull);
    }

    const auto write_size = std::min(Capacity - current_size_, size);

    for (size_t i = 0; i < write_size; ++i) {
      buffer_[current_size_ + i] = std::move(data[i]);
    }

    current_size_ += write_size;
    return write_size;
  }

  std::expected<std::size_t, Error> read_impl(T* data, std::size_t size) {
    if (empty()) {
      return std::unexpected(Error::BufferEmpty);
    }

    const auto read_size = std::min(current_size_, size);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memcpy(data, buffer_.data(), read_size * sizeof(T));
      // Move remaining data to front
      if (read_size < current_size_) {
        std::memmove(buffer_.data(), buffer_.data() + read_size,
                     (current_size_ - read_size) * sizeof(T));
      }
    } else {
      for (size_t i = 0; i < read_size; ++i) {
        data[i] = buffer_[i];
      }
      // Move remaining data to front
      for (size_t i = 0; i < current_size_ - read_size; ++i) {
        buffer_[i] = std::move(buffer_[i + read_size]);
      }
    }

    current_size_ -= read_size;
    return read_size;
  }

  void clear_impl() noexcept {
    current_size_ = 0;
    buffer_ = {};
  }

  size_t current_size_{0};
  alignas(T) std::array<T, Capacity> buffer_{};
  [[no_unique_address]] mutable mutex_type mutex_;
};

}  // namespace malib