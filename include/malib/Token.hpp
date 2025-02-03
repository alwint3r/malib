#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

#include "malib/Error.hpp"

namespace malib {
struct Token {
  const char* ptr{nullptr};
  size_t start{0};
  size_t end{0};

  std::expected<std::string_view, Error> view() const;
};
}  // namespace malib