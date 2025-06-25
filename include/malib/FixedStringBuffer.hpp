#pragma once

#include <array>
#include <cstring>

#include "malib/Error.hpp"
#include "malib/concepts.hpp"

namespace malib {
template <std::size_t MaxSize>
struct FixedStringBuffer {
  using value_type = char;
  using container_type = std::array<char, MaxSize>;
  using iterator = decltype(std::declval<container_type>().begin());
  using const_iterator = decltype(std::declval<container_type>().cbegin());

  FixedStringBuffer() = default;

  constexpr FixedStringBuffer(const char* str) {
    if (str) {
      std::size_t len = 0;
      while (len < MaxSize && str[len] != '\0') {
        buf_[len] = str[len];
        ++len;
      }
      size_ = len;
    } else {
      size_ = 0;
    }
  }

  Error copy(std_string auto value) {
    if constexpr (std::is_same_v<std::remove_cv_t<
                                     std::remove_reference_t<decltype(value)>>,
                                 std::string>) {
      return copy(value.c_str(), value.size());
    } else {
      return copy(value.data(), value.size());
    }
  }

  Error copy(c_str auto value, std::size_t size) {
    if (value == nullptr) {
      return Error::NullPointerInput;
    }

    if (size > MaxSize) {
      return Error::MaximumSizeExceeded;
    }

    std::memcpy(buf_.data(), value, size);
    size_ = size;
    return Error::Ok;
  }

  std::string_view view() const noexcept {
    return std::string_view{buf_.data(), size_};
  }

  void reset() {
    size_ = 0;
    buf_ = {0};
  }

  void clear() { reset(); }

  [[nodiscard]] iterator begin() noexcept { return buf_.begin(); }
  [[nodiscard]] iterator end() noexcept { return buf_.begin() + size_; }

  [[nodiscard]] const_iterator cbegin() const noexcept { return buf_.cbegin(); }
  [[nodiscard]] const_iterator cend() const noexcept {
    return buf_.cbegin() + size_;
  }

  [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }
  [[nodiscard]] const_iterator end() const noexcept { return cend(); }

  [[nodiscard]] iterator data() noexcept { return begin(); }

  [[nodiscard]] std::size_t size() const noexcept { return size_; }

  [[nodiscard]] std::size_t capacity() const noexcept { return MaxSize; }

  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

  [[nodiscard]] bool full() const noexcept { return size_ == MaxSize; }

  [[nodiscard]] char& operator[](std::size_t index) { return buf_[index]; }

  [[nodiscard]] const char& operator[](std::size_t index) const {
    return buf_[index];
  }

  [[nodiscard]] bool operator==(const FixedStringBuffer& other) const {
    return view() == other.view();
  }

  [[nodiscard]] bool operator!=(const FixedStringBuffer& other) const {
    return view() != other.view();
  }

  std::expected<std::size_t, Error> write(const char* data, std::size_t size) {
    if (data == nullptr) {
      return std::unexpected(Error::NullPointerInput);
    }

    if (size + size_ > MaxSize) {
      return std::unexpected(Error::MaximumSizeExceeded);
    }

    if (size > 1) {
      std::memcpy(buf_.data() + size_, data, size);
      size_ += size;
    } else {
      buf_[size_] = *data;
      ++size_;
    }

    return size;
  }

  std::expected<std::size_t, Error> write(std::string_view str) {
    if (str.size() + size_ > MaxSize) {
      return std::unexpected(Error::MaximumSizeExceeded);
    }

    std::memcpy(buf_.data() + size_, str.data(), str.size());
    size_ += str.size();
    return str.size();
  }

 private:
  std::array<char, MaxSize> buf_{0};
  std::size_t size_{0};
};

static_assert(raw_accessible<FixedStringBuffer<256>>);
static_assert(sizeof(FixedStringBuffer<32>) == (32 + sizeof(std::size_t)));
static_assert(sizeof(FixedStringBuffer<8>) == (8 + sizeof(std::size_t)));
}  // namespace malib
