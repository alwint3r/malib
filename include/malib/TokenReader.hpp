#pragma once

#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <string>
#include <type_traits>

#include "malib/Token.hpp"

namespace malib {
struct TokenReader {
  static Error to_number(const Token& token, std::integral auto& value,
                         int base = 10) {
    auto token_sv = token.view().value();
    auto [ptr, ec] = std::from_chars(
        token_sv.data(), token_sv.data() + token_sv.size(), value, base);

    if (ec == std::errc::invalid_argument) {
      return Error::InvalidArgument;
    } else if (ec == std::errc::result_out_of_range) {
      return Error::ResultOutOfRange;
    }

    return Error::Ok;
  }

  static Error to_number(const Token& token, std::floating_point auto& value) {
    auto token_sv = token.view().value();
    std::string token_str{token_sv.data(), token_sv.size()};
    char* end = nullptr;

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, float>) {
      value = std::strtof(token_str.c_str(), &end);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
      value = std::strtod(token_str.c_str(), &end);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, long double>) {
      value = std::strtold(token_str.c_str(), &end);
    }

    if (end == token_str.c_str()) {
      return Error::InvalidArgument;
    }

    return Error::Ok;
  }
};
};  // namespace malib