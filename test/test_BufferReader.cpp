#include <unity.h>

#include "malib/BufferReader.hpp"
#include "malib/RingBuffer.hpp"

void test_readAll() {
  using RingBuffer = malib::RingBuffer<char, 3>;
  RingBuffer buffer{};
  buffer.push('a');
  buffer.push('b');
  buffer.push('c');

  char buf[3] = {};
  auto result = malib::BufferReader::readAll(buffer, buf, 3);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_STRING("abc", buf);

  buffer.push('d');
  buffer.push('e');
  buffer.push('f');
  result = malib::BufferReader::readAll(buffer, buf, 5);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_STRING("def", buf);
  TEST_ASSERT_EQUAL(3, result.value());
}

void test_readAll_emptyBuffer() {
  using RingBuffer = malib::RingBuffer<char, 3>;
  RingBuffer buffer{};

  char buf[3] = {};
  auto result = malib::BufferReader::readAll(buffer, buf, 3);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferEmpty, result.error());
}

void test_readAll_nullPointer() {
  using RingBuffer = malib::RingBuffer<char, 3>;
  RingBuffer buffer{};
  buffer.push('a');

  auto result = malib::BufferReader::readAll(buffer, nullptr, 3);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::NullPointerOutput, result.error());
}

void test_readUntil() {
  using RingBuffer = malib::RingBuffer<char, 4>;
  RingBuffer buffer{};
  buffer.push('a');
  buffer.push('b');
  buffer.push('c');
  buffer.push('d');

  char buf[4] = {};
  auto result = malib::BufferReader::readUntil(buffer, 'c', buf, 3);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_STRING("abc", buf);
  TEST_ASSERT_EQUAL(3, result.value());
}

void test_readUntil_valueNotFound() {
  using RingBuffer = malib::RingBuffer<char, 4>;
  RingBuffer buffer{};
  buffer.push('a');
  buffer.push('b');
  buffer.push('c');
  buffer.push('d');

  char buf[4] = {};
  auto result = malib::BufferReader::readUntil(buffer, 'e', buf, 4);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_STRING("abcd", buf);
  TEST_ASSERT_EQUAL(4, result.value());
}

void test_readUntil_nullPointer() {
  using RingBuffer = malib::RingBuffer<char, 4>;
  RingBuffer buffer{};
  buffer.push('a');

  auto result = malib::BufferReader::readUntil(buffer, 'a', nullptr, 4);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::NullPointerOutput, result.error());
}

void test_readUntil_emptyBuffer() {
  using RingBuffer = malib::RingBuffer<char, 4>;
  RingBuffer buffer{};

  char buf[4] = {};
  auto result = malib::BufferReader::readUntil(buffer, 'a', buf, 4);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferEmpty, result.error());
}

void test_BufferReader() {
  RUN_TEST(test_readAll);
  RUN_TEST(test_readUntil);
  RUN_TEST(test_readAll_emptyBuffer);
  RUN_TEST(test_readAll_nullPointer);
  RUN_TEST(test_readUntil_valueNotFound);
  RUN_TEST(test_readUntil_nullPointer);
  RUN_TEST(test_readUntil_emptyBuffer);
}