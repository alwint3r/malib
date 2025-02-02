#pragma once

#include <concepts>
#include <variant>

#include "malib/Error.hpp"
#include "malib/RingBuffer.hpp"

namespace malib {
template <typename B>
concept buffer_like = requires(B t) {
  { t.push(std::declval<typename B::ValueType>()) } -> std::same_as<Error>;
  { t.pop() } -> std::same_as<std::expected<typename B::ValueType, Error>>;
  { t.peek() } -> std::same_as<std::expected<typename B::ValueType, Error>>;
  { t.size() } -> std::same_as<size_t>;
  { t.empty() } -> std::same_as<bool>;
  { t.clear() } -> std::same_as<void>;
  { std::tuple_size_v<B> } -> std::convertible_to<std::size_t>;
};

static_assert(buffer_like<RingBuffer<int, 0>>);

template <typename B>
  requires buffer_like<B>
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