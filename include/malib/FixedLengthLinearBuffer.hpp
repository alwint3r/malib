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
  // Forward declare the sizing_iterator as a private inner class
  class sizing_iterator;
  friend class sizing_iterator;

  static_assert(Capacity > 0);
  using value_type = T;
  using mutex_type = std::mutex;
  using lock_guard = std::lock_guard<mutex_type>;

 public:
  FixedLengthLinearBuffer() noexcept = default;
  ~FixedLengthLinearBuffer() noexcept = default;
  FixedLengthLinearBuffer(const FixedLengthLinearBuffer&) = delete;
  FixedLengthLinearBuffer& operator=(const FixedLengthLinearBuffer&) = delete;
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

  // Keep the format_begin/end methods public
  sizing_iterator format_begin() noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return sizing_iterator(buffer_.data(), this);
    }
    return sizing_iterator(buffer_.data(), this);
  }

  sizing_iterator format_end() noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      return sizing_iterator(buffer_.data() + Capacity, this);
    }
    return sizing_iterator(buffer_.data() + Capacity, this);
  }

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

  // Move set_size to private section
  void set_size(size_t new_size) noexcept {
    if constexpr (ThreadSafe) {
      lock_guard lock(mutex_);
      current_size_ = std::min(new_size, Capacity);
    } else {
      current_size_ = std::min(new_size, Capacity);
    }
  }

  size_t current_size_{0};
  alignas(T) std::array<T, Capacity> buffer_{};
  [[no_unique_address]] mutable mutex_type mutex_;

  // Move sizing_iterator definition here but keep it private
  class sizing_iterator {
   public:
    // Iterator traits
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    sizing_iterator(pointer p, FixedLengthLinearBuffer* buffer)
        : ptr_(p), buffer_(buffer) {}

    // Assignment updates buffer size
    sizing_iterator& operator=(const sizing_iterator& other) {
      ptr_ = other.ptr_;
      if (buffer_) {
        buffer_->set_size(std::distance(buffer_->buffer_.data(), ptr_));
      }
      return *this;
    }

    // Required iterator operations
    reference operator*() { return *ptr_; }
    pointer operator->() { return ptr_; }

    // Increment/decrement
    sizing_iterator& operator++() {
      ++ptr_;
      if (buffer_) {
        buffer_->set_size(std::distance(buffer_->buffer_.data(), ptr_));
      }
      return *this;
    }

    sizing_iterator operator++(int) {
      sizing_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    // Arithmetic operations
    sizing_iterator& operator+=(difference_type n) {
      ptr_ += n;
      if (buffer_) {
        buffer_->set_size(std::distance(buffer_->buffer_.data(), ptr_));
      }
      return *this;
    }

    friend sizing_iterator operator+(sizing_iterator it, difference_type n) {
      return it += n;
    }

    friend sizing_iterator operator+(difference_type n, sizing_iterator it) {
      return it += n;
    }

    // Comparison operators
    friend bool operator==(const sizing_iterator& a, const sizing_iterator& b) {
      return a.ptr_ == b.ptr_;
    }

    friend bool operator!=(const sizing_iterator& a, const sizing_iterator& b) {
      return !(a == b);
    }

   private:
    pointer ptr_;
    FixedLengthLinearBuffer* buffer_;
  };
};

}  // namespace malib