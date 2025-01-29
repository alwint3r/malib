#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

namespace malib {
struct Token {
  const char* ptr;
  size_t start;
  size_t end;

  enum class Error : std::uint8_t {
    NullPointerData,
    InvalidSize,
  };

  std::expected<std::string_view, Error> view() const;
};
}  // namespace malib