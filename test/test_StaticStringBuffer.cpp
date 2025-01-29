#include <unity.h>

#include "StaticStringBuffer.hpp"

void test_static_string_buffer_copy() {
  using SmallBuffer = malib::StaticStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd1, 5));

  std::string_view view = buffer.view();
  TEST_ASSERT_EQUAL_STRING("Hello", view.data());

  // exceed max size test
  std::string_view cmd2 = "abcdefghijklmn";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::MaxSizeExceeded, buffer.copy(cmd2));

  std::string cmd3{"abcdefghijklmn"};
  TEST_ASSERT_EQUAL(SmallBuffer::Error::MaxSizeExceeded, buffer.copy(cmd3));

  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());

  // null pointer input
  const char* cmd4 = NULL;
  TEST_ASSERT_EQUAL(SmallBuffer::Error::NullPointerInput, buffer.copy(cmd4, 1));
}
