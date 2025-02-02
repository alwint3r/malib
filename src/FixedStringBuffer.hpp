#pragma once

#include "MalibError.hpp"
#include "StringConcepts.hpp"

namespace malib {
template <std::size_t MaxSize>
struct FixedStringBuffer {
  [[nodiscard]] Error copy(StringLike auto value) {
    if constexpr (std::is_same_v<std::remove_cv_t<
                                     std::remove_reference_t<decltype(value)>>,
                                 std::string>) {
      return copy(value.c_str(), value.size());
    } else {
      return copy(value.data(), value.size());
    }
  }
  [[nodiscard]] Error copy(CStyleString auto value, std::size_t size) {
    if (value == nullptr) {
      return Error::NullPointerInput;
    }

    if (size > MaxSize) {
      return Error::MaximumSizeExceeded;
    }

    std::memcpy(buf_, value, size);
    size_ = size;
    return Error::Ok;
  }

  std::string_view view() const noexcept {
    return std::string_view{buf_, size_};
  }

  void reset() {
    size_ = 0;
    *buf_ = {};
  }

 private:
  char buf_[MaxSize]{0};
  std::size_t size_{0};
};
}  // namespace malib
