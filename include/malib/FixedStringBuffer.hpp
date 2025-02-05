#pragma once

#include <array>

#include "malib/Error.hpp"
#include "malib/concepts.hpp"

namespace malib {
template <std::size_t MaxSize>
struct FixedStringBuffer {
  using container_type = std::array<char, MaxSize>;
  using iterator = decltype(std::declval<container_type>().begin());
  using const_iterator = decltype(std::declval<container_type>().cbegin());

  Error copy(string_like auto value) {
    if constexpr (std::is_same_v<std::remove_cv_t<
                                     std::remove_reference_t<decltype(value)>>,
                                 std::string>) {
      return copy(value.c_str(), value.size());
    } else {
      return copy(value.data(), value.size());
    }
  }

  Error copy(cstyle_string auto value, std::size_t size) {
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

  [[nodiscard]] iterator begin() noexcept { return buf_.begin(); }
  [[nodiscard]] iterator end() noexcept { return buf_.begin() + size_; }

  [[nodiscard]] const_iterator cbegin() const noexcept { return buf_.cbegin(); }
  [[nodiscard]] const_iterator cend() const noexcept {
    return buf_.cbegin() + size_;
  }

  [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }
  [[nodiscard]] const_iterator end() const noexcept { return cend(); }

  [[nodiscard]] const_iterator data() const noexcept { return cbegin(); }

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

 private:
  std::array<char, MaxSize> buf_{0};
  std::size_t size_{0};
};
}  // namespace malib
