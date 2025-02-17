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

  /**
   * Reads bytes from source until a delimiter is found or source is empty,
   * writing all read bytes (including delimiter) to destination.
   *
   * @param source The source container to read from
   * @param delimiter The value to stop reading at (inclusive)
   * @param destination The destination to write bytes to
   *
   * @return Number of bytes read and written on success, or an error
   */
  template <typename B, typename C>
    requires((poppable_container<B> and container_like<B>) and
             (byte_output_interface<C>))
  static std::expected<std::size_t, Error> readUntil(
      B& source, typename B::value_type delimiter, C& destination) {
    if (source.empty()) {
      return std::unexpected(Error::BufferEmpty);
    }

    std::size_t count = 0;

    while (!source.empty()) {
      auto result = source.pop();
      if (!result.has_value()) {
        return std::unexpected(result.error());
      }

      auto value = result.value();
      auto write_result = destination.write(&value, 1);
      if (!write_result.has_value()) {
        return std::unexpected(write_result.error());
      }

      count++;
      if (value == delimiter) {
        break;
      }
    }

    return count;
  }
};
}  // namespace malib