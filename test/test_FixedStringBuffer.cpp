#include <unity.h>

#include "FixedStringBuffer.hpp"

void test_fixed_string_buffer_copy() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd1, 5));
  TEST_ASSERT_EQUAL(5, buffer.view().size());
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  // exceed max size test
  std::string_view cmd2 = "abcdefghijklmn";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::MaxSizeExceeded, buffer.copy(cmd2));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  std::string cmd3{"abcdefghijklmn"};
  TEST_ASSERT_EQUAL(SmallBuffer::Error::MaxSizeExceeded, buffer.copy(cmd3));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  // null pointer input
  const char* cmd4 = NULL;
  TEST_ASSERT_EQUAL(SmallBuffer::Error::NullPointerInput, buffer.copy(cmd4, 1));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);
}

void test_fixed_string_buffer_reset() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd1, 5));

  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_fixed_string_buffer_copy_empty_string() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd, 0));
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_fixed_string_buffer_copy_exact_max_size() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "1234567890";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::Ok, buffer.copy(cmd, 10));
  TEST_ASSERT_EQUAL_STRING_LEN("1234567890", buffer.view().data(), 10);
}

void test_fixed_string_buffer_reset_after_exceeding_max_size() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "12345678901";
  TEST_ASSERT_EQUAL(SmallBuffer::Error::MaxSizeExceeded, buffer.copy(cmd, 11));
  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_FixedStringBuffer() {
  RUN_TEST(test_fixed_string_buffer_copy);
  RUN_TEST(test_fixed_string_buffer_reset);
  RUN_TEST(test_fixed_string_buffer_copy_empty_string);
  RUN_TEST(test_fixed_string_buffer_copy_exact_max_size);
  RUN_TEST(test_fixed_string_buffer_reset_after_exceeding_max_size);
}