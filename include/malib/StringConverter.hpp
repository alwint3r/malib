#pragma once

#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>

#include "malib/Token.hpp"

namespace malib {
struct StringConverter {  // Changed from TokenReader
  static Error to_number(std::string_view input, std::integral auto& value,
                         int radix = 10) {
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(),
                                     value, radix);

    if (ec == std::errc::invalid_argument) {
      return Error::InvalidArgument;
    } else if (ec == std::errc::result_out_of_range) {
      return Error::ResultOutOfRange;
    }

    return Error::Ok;
  }

  static Error to_number(std::string_view input,
                         std::floating_point auto& value) {
    std::string input_str{input.data(), input.size()};
    char* end = nullptr;

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, float>) {
      value = std::strtof(input_str.c_str(), &end);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
      value = std::strtod(input_str.c_str(), &end);
    }

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, long double>) {
      value = std::strtold(input_str.c_str(), &end);
    }

    if (end == input_str.c_str()) {
      return Error::InvalidArgument;
    }

    return Error::Ok;
  }
};
}  // namespace malib