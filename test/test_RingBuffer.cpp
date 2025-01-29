#include <unity.h>

#include "RingBuffer.hpp"

void test_push_pop() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::OK, buffer.push(1));
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::OK, buffer.push(2));
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::OK, buffer.push(3));
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::Full, buffer.push(4));

  auto result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());

  result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(2, result.value());

  result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());

  result = buffer.pop();
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::Empty, result.error());
}

void test_clear() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  buffer.push(1);
  buffer.push(2);
  buffer.clear();
  TEST_ASSERT_TRUE(buffer.empty());
  TEST_ASSERT_EQUAL(SmallRingBuffer::Error::OK, buffer.push(3));
}

void test_size() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  TEST_ASSERT_EQUAL(0, buffer.size());
  buffer.push(1);
  TEST_ASSERT_EQUAL(1, buffer.size());
  buffer.push(2);
  TEST_ASSERT_EQUAL(2, buffer.size());
  buffer.pop();
  TEST_ASSERT_EQUAL(1, buffer.size());
}

void test_RingBuffer() {
  RUN_TEST(test_push_pop);
  RUN_TEST(test_clear);
  RUN_TEST(test_size);
}