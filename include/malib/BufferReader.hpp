#pragma once

#include <concepts>
#include <variant>

#include "malib/Error.hpp"
#include "malib/RingBuffer.hpp"
#include "malib/concepts.hpp"

namespace malib {
struct BufferReader {
  template <typename B>
    requires(poppable_container<B> and container_like<B>)
  static std::expected<std::size_t, Error> readAll(
      B& buffer, typename B::value_type* elements, std::size_t maxSize) {
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

  template <typename B>
    requires(poppable_container<B> and container_like<B>)
  static std::expected<std::size_t, Error> readUntil(
      B& buffer, typename B::value_type value, typename B::value_type* elements,
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