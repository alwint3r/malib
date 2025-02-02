#pragma once

#include <cstdint>

namespace malib {
enum class Error : std::uint32_t {
  Ok = 0,
  NullPointerMember,
  NullPointerInput,
  NullPointerOutput,
  BufferEmpty,
  BufferFull,
  InvalidSize,
  IndexOutOfRange,
  MaximumSizeExceeded,
};
};