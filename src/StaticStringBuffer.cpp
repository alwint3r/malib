#include "StaticStringBuffer.hpp"

using namespace malib;

template <std::size_t MaxSize>
StaticStringBuffer<MaxSize>::Error StaticStringBuffer<MaxSize>::copy(
    CStyleString auto &value, std::size_t size) {
  if (value == nullptr) {
    return Error::NullPointerInput;
  }

  if (size > MaxSize) {
    return Error::MaxSizeExceeded;
  }

  std::memcpy(buf_, value, size);
  size_ = size;
  return Error::Ok;
}

template <std::size_t MaxSize>
StaticStringBuffer<MaxSize>::Error StaticStringBuffer<MaxSize>::copy(
    StringLike auto &value) {
  if constexpr (std::is_same_v<
                    std::remove_cv_t<std::remove_reference_t<decltype(value)>>,
                    std::string>) {
    return copy(value.c_str(), value.size());
  } else {
    return copy(value.data(), value.size());
  }
}
