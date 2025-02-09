#include "malib/FixedStringBuffer.hpp"
#include "malib/RingBuffer.hpp"
#include "malib/Shell.hpp"

template class malib::FixedStringBuffer<10>;

struct test_output {
  std::expected<std::size_t, malib::Error> write(const char* buf,
                                                 std::size_t size) {
    return size;
  }
};

static_assert(malib::byte_output_interface<test_output>);