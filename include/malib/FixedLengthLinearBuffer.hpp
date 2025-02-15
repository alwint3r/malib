#pragma once

#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <type_traits>

#include "malib/Error.hpp"

namespace malib {
template <typename T, size_t Capacity>
  requires std::copyable<T>
class FixedLengthLinearBuffer {
  static_assert(Capacity > 0);
  using value_type = T;

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
    if (full()) {
      return std::unexpected(Error::BufferFull);
    }

    const auto write_size = std::min(free_space(), size);

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

  template <typename U = T>
    requires(!std::is_trivially_copyable_v<U>)
  std::expected<std::size_t, Error> write_move(U* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }
    if (full()) {
      return std::unexpected(Error::BufferFull);
    }

    const auto write_size = std::min(free_space(), size);

    for (size_t i = 0; i < write_size; ++i) {
      buffer_[current_size_ + i] = std::move(data[i]);
    }

    current_size_ += write_size;
    return write_size;
  }

  std::expected<std::size_t, Error> read(T* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }
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

  [[nodiscard]] size_t size() const noexcept { return current_size_; }
  [[nodiscard]] size_t capacity() const noexcept { return Capacity; }
  [[nodiscard]] size_t free_space() const noexcept {
    return Capacity - current_size_;
  }
  [[nodiscard]] bool empty() const noexcept { return current_size_ == 0; }
  [[nodiscard]] bool full() const noexcept { return current_size_ == Capacity; }

  void clear() noexcept {
    current_size_ = 0;
    buffer_ = {};
  }

  const T* data() const noexcept { return buffer_.data(); }
  T* data() noexcept { return buffer_.data(); }

 private:
  size_t current_size_{0};
  alignas(T) std::array<T, Capacity> buffer_{};
};
}  // namespace malib