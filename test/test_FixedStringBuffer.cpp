#include <unity.h>

#include <format>

#include "malib/FixedStringBuffer.hpp"

void test_fixed_string_buffer_copy() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.copy(cmd1, 5));
  TEST_ASSERT_EQUAL(5, buffer.view().size());
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  // exceed max size test
  std::string_view cmd2 = "abcdefghijklmn";
  TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, buffer.copy(cmd2));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  std::string cmd3{"abcdefghijklmn"};
  TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, buffer.copy(cmd3));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);

  // null pointer input
  const char* cmd4 = NULL;
  TEST_ASSERT_EQUAL(malib::Error::NullPointerInput, buffer.copy(cmd4, 1));
  TEST_ASSERT_EQUAL(5, buffer.view().size());  // size should remain unchanged
  TEST_ASSERT_EQUAL_STRING_LEN("Hello", buffer.view().data(), 5);
}

void test_fixed_string_buffer_reset() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd1 = "Hello";
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.copy(cmd1, 5));

  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_fixed_string_buffer_copy_empty_string() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "";
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.copy(cmd, 0));
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_fixed_string_buffer_copy_exact_max_size() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "1234567890";
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.copy(cmd, 10));
  TEST_ASSERT_EQUAL_STRING_LEN("1234567890", buffer.view().data(), 10);
}

void test_fixed_string_buffer_reset_after_exceeding_max_size() {
  using SmallBuffer = malib::FixedStringBuffer<10>;
  SmallBuffer buffer{};
  const char* cmd = "12345678901";
  TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, buffer.copy(cmd, 11));
  buffer.reset();
  TEST_ASSERT_EQUAL(0, buffer.view().size());
}

void test_fixed_string_buffer_comparison_operators() {
    using SmallBuffer = malib::FixedStringBuffer<10>;
    SmallBuffer buffer1{};
    SmallBuffer buffer2{};
    
    buffer1.copy("test", 4);
    buffer2.copy("test", 4);
    TEST_ASSERT_TRUE(buffer1 == buffer2);
    
    buffer2.copy("test2", 5);
    TEST_ASSERT_TRUE(buffer1 != buffer2);
}

void test_fixed_string_buffer_iterators() {
    using SmallBuffer = malib::FixedStringBuffer<10>;
    SmallBuffer buffer{};
    buffer.copy("test", 4);
    
    std::string result;
    for(auto it = buffer.begin(); it != buffer.end(); ++it) {
        result += *it;
    }
    TEST_ASSERT_EQUAL_STRING("test", result.c_str());
    
    result.clear();
    for(const auto& c : buffer) {
        result += c;
    }
    TEST_ASSERT_EQUAL_STRING("test", result.c_str());
}

void test_fixed_string_buffer_array_access() {
    using SmallBuffer = malib::FixedStringBuffer<10>;
    SmallBuffer buffer{};
    buffer.copy("test", 4);
    
    TEST_ASSERT_EQUAL('t', buffer[0]);
    TEST_ASSERT_EQUAL('e', buffer[1]);
    TEST_ASSERT_EQUAL('s', buffer[2]);
    TEST_ASSERT_EQUAL('t', buffer[3]);
}

void test_fixed_string_buffer_state() {
    using SmallBuffer = malib::FixedStringBuffer<3>;
    SmallBuffer buffer{};
    
    TEST_ASSERT_TRUE(buffer.empty());
    TEST_ASSERT_FALSE(buffer.full());
    
    buffer.copy("ab", 2);
    TEST_ASSERT_FALSE(buffer.empty());
    TEST_ASSERT_FALSE(buffer.full());
    
    buffer.copy("abc", 3);
    TEST_ASSERT_FALSE(buffer.empty());
    TEST_ASSERT_TRUE(buffer.full());
}

void test_fixed_string_buffer_format() {
    using SmallBuffer = malib::FixedStringBuffer<20>;
    SmallBuffer buffer{};
    
    std::format_to(buffer.begin(), "Test {}", 42);
    TEST_ASSERT_EQUAL_STRING("Test 42", buffer.view().data());
    
    buffer.reset();
    std::format_to(buffer.begin(), "{:.2f}", 3.14159);
    TEST_ASSERT_EQUAL_STRING("3.14", buffer.view().data());
    
    // Test overflow protection
    buffer.reset();
    auto result = std::format_to_n(buffer.begin(), buffer.capacity(), 
        "Very long string that exceeds buffer size: {}", 12345);
    TEST_ASSERT_LESS_OR_EQUAL(buffer.capacity(), result.out - buffer.begin());
}

void test_FixedStringBuffer() {
  RUN_TEST(test_fixed_string_buffer_copy);
  RUN_TEST(test_fixed_string_buffer_reset);
  RUN_TEST(test_fixed_string_buffer_copy_empty_string);
  RUN_TEST(test_fixed_string_buffer_copy_exact_max_size);
  RUN_TEST(test_fixed_string_buffer_reset_after_exceeding_max_size);
  RUN_TEST(test_fixed_string_buffer_comparison_operators);
  RUN_TEST(test_fixed_string_buffer_iterators);
  RUN_TEST(test_fixed_string_buffer_array_access);
  RUN_TEST(test_fixed_string_buffer_state);
  RUN_TEST(test_fixed_string_buffer_format);
}