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
      markers_[count_].offset = pos;

      while (pos < size) {
        char c = buf[pos];
        if (isQuote(c)) {
          quoted = !quoted;
        }

        if (std::isspace(c) && !quoted) {
          markers_[count_].length = pos - markers_[count_].offset;
          scanning = false;
          count_++;
          pos++;
          break;
        }
        pos++;
      }
    }

    if (scanning) {
      markers_[count_].length = pos - markers_[count_].offset;
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

  std::vector<std::string_view> tokens_vector(
      std::string_view input) const noexcept {
    std::vector<std::string_view> tokens;
    tokens.reserve(count_);
    for (std::size_t i = 0; i < count_; i++) {
      tokens.push_back(markers_[i].view(input));
    }
    return tokens;
  }

  std::span<const Token> tokens_span() const noexcept {
    return std::span<const Token>(markers_.data(), count_);
  }

  auto tokens_views(std::string_view input) const noexcept {
    return TokenViews{input, tokens_span()};
  }

 private:
  bool isQuote(char c) { return c == '\'' || c == '"'; }

 private:
  std::array<Token, MaxTokens> markers_{0};
  std::size_t count_{0};
};

}  // namespace malib