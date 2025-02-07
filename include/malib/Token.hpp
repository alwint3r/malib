#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
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

/**
 * @struct TokenViews
 * @brief A view container that provides access to a sequence of tokens within a base string.
 *
 * TokenViews provides a convenient way to iterate over and access token contents
 * from a base string. It holds a reference to the base string and a span of Token
 * objects, allowing efficient access to token substrings without copying data.
 */
struct TokenViews {
  std::string_view base;     ///< The base string containing the token contents
  std::span<const Token> tokens;  ///< Span of tokens representing substrings within the base string

  /**
   * @struct TokenIterator
   * @brief Iterator for traversing token contents within a TokenViews container.
   *
   * Provides standard iterator interface for accessing token contents directly
   * as string_views, hiding the token offset/length implementation details.
   */
  struct TokenIterator {
    std::string_view base;              ///< Reference to the base string
    std::span<const Token>::iterator current;  ///< Current position in the token span

    std::string_view operator*() const noexcept { return current->view(base); }

    TokenIterator& operator++() noexcept {
      ++current;
      return *this;
    }

    TokenIterator operator++(int) noexcept {
      TokenIterator tmp = *this;
      ++current;
      return tmp;
    }

    bool operator!=(const TokenIterator& other) const noexcept {
      return current != other.current;
    }

    bool operator==(const TokenIterator& other) const noexcept {
      return current == other.current;
    }
  };

  /**
   * @brief Returns an iterator to the beginning of the token sequence.
   * @return TokenIterator pointing to the first token.
   */
  TokenIterator begin() const { return TokenIterator{base, tokens.begin()}; }

  /**
   * @brief Returns an iterator to the end of the token sequence.
   * @return TokenIterator pointing past the last token.
   */
  TokenIterator end() const { return TokenIterator{base, tokens.end()}; }

  /**
   * @brief Returns the number of tokens in the sequence.
   * @return Size of the token sequence.
   */
  std::size_t size() const noexcept { return tokens.size(); }

  /**
   * @brief Checks if the token sequence is empty.
   * @return true if there are no tokens, false otherwise.
   */
  bool empty() const noexcept { return tokens.empty(); }

  /**
   * @brief Accesses a token's contents by index.
   * @param idx Index of the token to access.
   * @return Expected containing the token's contents as string_view if idx is valid,
   *         or Error::IndexOutOfRange if idx is out of bounds.
   */
  std::expected<std::string_view, Error> operator[](
      std::size_t idx) const noexcept {
    if (idx >= tokens.size()) {
      return std::unexpected{Error::IndexOutOfRange};
    }
    return tokens[idx].view(base);
  }
};
}  // namespace malib