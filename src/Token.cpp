#include "malib/Token.hpp"

std::expected<std::string_view, malib::Error> malib::Token::view() const {
  if (ptr == nullptr) {
    return std::unexpected(Error::NullPointerMember);
  }

  if (end < start) {
    return std::unexpected(Error::InvalidSize);
  }

  const char* start_ptr = ptr + start;
  size_t size = (end - start) + 1;
  return std::string_view(start_ptr, size);
}