#pragma once

#include "StringConcepts.hpp"

namespace malib {
template <std::size_t MaxSize>
struct StaticStringBuffer {
  enum class Error {
    Ok,
    MaxSizeExceeded,
    NullPointerInput,
  };

  [[nodiscard]] Error copy(StringLike auto &value);
  [[nodiscard]] Error copy(CStyleString auto &value, std::size_t size);

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
