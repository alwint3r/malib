#include <unity.h>

#include "FixedStringBuffer.hpp"

void test_fixed_string_buffer_copy() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
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

  // null pointer input
  const char* cmd4 = NULL;
  TEST_ASSERT_EQUAL(SmallBuffer::Error::NullPointerInput, buffer.copy(cmd4, 1));
}

void test_fixed_string_buffer_reset() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd1, 5));

  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}