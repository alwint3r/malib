#pragma once

#include <array>
#include <cstddef>
#include <expected>
#include <string_view>
#include <vector>

#include "malib/Error.hpp"
#include "malib/Token.hpp"

namespace malib {
template <std::size_t MaxTokens>
class Tokenizer {
 public:
  std::expected<std::size_t, Error> tokenize(std::string_view str) {
    count_ = 0;
    std::size_t pos = 0;
    bool quoted = false;
    bool scanning = false;

    auto size = str.size();
    auto buf = str.data();

    while (pos < size) {
      if (count_ >= MaxTokens) {
        return std::unexpected(Error::MaximumSizeExceeded);
      }

      while (std::isspace(buf[pos])) {
        pos++;
      }

      scanning = true;
      markers_[count_].start = pos;
      markers_[count_].ptr = buf;

      while (pos < size) {
        char c = buf[pos];
        if (isQuote(c)) {
          quoted = !quoted;
        }

        if (std::isspace(c) && !quoted) {
          markers_[count_].end = pos - 1;
          scanning = false;
          count_++;
          pos++;
          break;
        }
        pos++;
      }
    }

    if (scanning) {
      markers_[count_].end = pos - 1;
      count_++;
    }

    return count_;
  }

  std::expected<Token, Error> operator[](std::size_t idx) const noexcept {
    if (idx >= MaxTokens || idx >= count_) {
      return std::unexpected(Error::IndexOutOfRange);
    }
    return markers_[idx];
  }

  std::vector<Token> tokens_vector() const noexcept {
    std::vector<Token> tokens;
    tokens.reserve(count_);
    for (std::size_t i = 0; i < count_; i++) {
      tokens.push_back(markers_[i]);
    }
    return tokens;
  }

 private:
  bool isQuote(char c) { return c == '\'' || c == '"'; }

 private:
  std::array<Token, MaxTokens> markers_{0};
  std::size_t count_{0};
};

}  // namespace malib