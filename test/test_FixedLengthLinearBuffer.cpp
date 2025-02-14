#include <unity.h>

#include <malib/FixedLengthLinearBuffer.hpp>
#include <string>

void test_FixedLengthLinearBuffer_trivially_copyable_type() {
  malib::FixedLengthLinearBuffer<int, 4> buffer;

  TEST_ASSERT_TRUE(buffer.empty());
  TEST_ASSERT_FALSE(buffer.full());
  TEST_ASSERT_EQUAL(0, buffer.size());
  TEST_ASSERT_EQUAL(4, buffer.capacity());
  TEST_ASSERT_EQUAL(4, buffer.free_space());

  int data[] = {1, 2, 3};
  auto result = buffer.write(data, 3);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());
  TEST_ASSERT_EQUAL(3, buffer.size());
  TEST_ASSERT_EQUAL(1, buffer.free_space());

  // Test buffer full condition
  int more_data[] = {4, 5};
  result = buffer.write(more_data, 2);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());
  TEST_ASSERT_TRUE(buffer.full());

  // Test write to full buffer
  int extra_data[] = {6};
  result = buffer.write(extra_data, 1);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, result.error());

  buffer.clear();
  TEST_ASSERT_TRUE(buffer.empty());
  TEST_ASSERT_EQUAL(0, buffer.size());
}

void test_FixedLengthLinearBuffer_non_trivially_copyable_type() {
  malib::FixedLengthLinearBuffer<std::string, 3> buffer;

  std::string data[] = {"hello", "world", "test", "extra"};
  auto result = buffer.write_move(data, 4);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());
  TEST_ASSERT_TRUE(buffer.full());

  // Test write to full buffer
  std::string extra[] = {"overflow"};
  result = buffer.write_move(extra, 1);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, result.error());
}

void test_FixedLengthLinearBuffer_zero_size_write() {
  malib::FixedLengthLinearBuffer<int, 4> buffer;
  int data[] = {1};
  auto result = buffer.write(data, 0);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(0, result.value());
  TEST_ASSERT_TRUE(buffer.empty());
}

void test_FixedLengthLinearBuffer_edge_cases() {
  // Test null pointer
  malib::FixedLengthLinearBuffer<int, 4> buffer;
  int* null_data = nullptr;
  auto result = buffer.write(null_data, 1);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::NullPointerInput, result.error());

  // Test exact capacity write
  int data[4] = {1, 2, 3, 4};
  result = buffer.write(data, 4);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(4, result.value());
  TEST_ASSERT_TRUE(buffer.full());
}

void test_FixedLengthLinearBuffer() {
  RUN_TEST(test_FixedLengthLinearBuffer_trivially_copyable_type);
  RUN_TEST(test_FixedLengthLinearBuffer_non_trivially_copyable_type);
  RUN_TEST(test_FixedLengthLinearBuffer_zero_size_write);
  RUN_TEST(test_FixedLengthLinearBuffer_edge_cases);
}