#pragma once

#include <concepts>
#include <variant>

#include "malib/Error.hpp"
#include "malib/RingBuffer.hpp"
#include "malib/concepts.hpp"

namespace malib {
template <buffer_like B>
struct BufferReader {
  using ValueType = typename B::ValueType;

  static std::expected<std::size_t, Error> readAll(B& buffer,
                                                   ValueType* elements,
                                                   std::size_t maxSize) {
    if (elements == nullptr) {
      return std::unexpected(Error::NullPointerOutput);
    }

    if (buffer.empty()) {
      return std::unexpected(Error::BufferEmpty);
    }

    std::size_t count = 0;
    while (count < maxSize && !buffer.empty()) {
      auto result = buffer.pop();
      if (result.has_value()) {
        elements[count++] = result.value();
      } else {
        return result;
      }
    }
    return count;
  }

  static std::expected<std::size_t, Error> readUntil(B& buffer, ValueType value,
                                                     ValueType* elements,
                                                     std::size_t maxSize) {
    if (elements == nullptr) {
      return std::unexpected{Error::NullPointerOutput};
    }

    if (buffer.empty()) {
      return std::unexpected(Error::BufferEmpty);
    }

    std::size_t count = 0;
    while (count < maxSize && !buffer.empty()) {
      auto result = buffer.pop();
      if (result.has_value()) {
        elements[count++] = result.value();
        if (result.value() == value) {
          break;
        }
      } else {
        return result;
      }
    }

    return count;
  }
};
}  // namespace malib