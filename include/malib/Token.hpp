#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>

#include "malib/Error.hpp"

namespace malib {

/**
 * @struct Token
 * @brief Represents a token with an offset and length.
 *
 * The Token struct is used to represent a substring within a larger string.
 * It contains an offset and length, both packed into a 32-bit integer.
 * It must not overlap with other token and must be within the bounds of the
 * base string.
 */
struct Token {
  uint32_t offset : 24;  ///< The offset of the token within the base string (24
                         ///< bits).
  uint32_t length : 8;   ///< The length of the token (8 bits).

  /**
   * @brief Returns a view of the token within the base string.
   *
   * @param base The base string from which the token is derived.
   * @return A std::string_view representing the token.
   */
  std::string_view view(std::string_view base) const noexcept {
    return base.substr(offset, length);
  }

  /**
   * @brief Checks if the token is empty.
   *
   * @return true if the token length is 0, false otherwise.
   */
  bool empty() const noexcept { return length == 0; }
};

static_assert(sizeof(Token) == 4, "Token should be 4 bytes");

}  // namespace malib