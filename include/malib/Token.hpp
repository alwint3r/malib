#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

#include "malib/Error.hpp"

namespace malib {
struct Token {
  const char* ptr;
  size_t start;
  size_t end;

  std::expected<std::string_view, Error> view() const;
};
}  // namespace malib