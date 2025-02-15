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

void test_FixedLengthLinearBuffer_write_partial() {
  malib::FixedLengthLinearBuffer<int, 3> buffer;

  int data[] = {1, 2, 3, 4, 5};
  auto result = buffer.write(data, 5);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());
  TEST_ASSERT_TRUE(buffer.full());
}

void test_FixedLengthLinearBuffer_write_non_trivial() {
  malib::FixedLengthLinearBuffer<std::string, 3> buffer;

  std::string data[] = {"copy", "these", "strings"};
  auto result = buffer.write(data, 3);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());
  TEST_ASSERT_TRUE(buffer.full());

  // Verify the original strings weren't moved
  TEST_ASSERT_EQUAL_STRING("copy", data[0].c_str());
  TEST_ASSERT_EQUAL_STRING("these", data[1].c_str());
  TEST_ASSERT_EQUAL_STRING("strings", data[2].c_str());
}

void test_FixedLengthLinearBuffer_append_one() {
  malib::FixedLengthLinearBuffer<int, 2> buffer;

  // Test append when buffer is not full
  int val1 = 42;
  auto result = buffer.write(&val1, 1);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());
  TEST_ASSERT_EQUAL(1, buffer.size());
  TEST_ASSERT_FALSE(buffer.full());

  // Test append when buffer has exactly one space left
  int val2 = 43;
  result = buffer.write(&val2, 1);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());
  TEST_ASSERT_EQUAL(2, buffer.size());
  TEST_ASSERT_TRUE(buffer.full());

  // Test append when buffer is full
  int val3 = 44;
  result = buffer.write(&val3, 1);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, result.error());
}

void test_FixedLengthLinearBuffer_read_write() {
  malib::FixedLengthLinearBuffer<int, 4> buffer;

  // Test reading from empty buffer
  int read_data[4];
  auto read_result = buffer.read(read_data, 4);
  TEST_ASSERT_FALSE(read_result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferEmpty, read_result.error());

  // Test write then read
  int write_data[] = {1, 2, 3};
  auto write_result = buffer.write(write_data, 3);
  TEST_ASSERT_TRUE(write_result.has_value());
  TEST_ASSERT_EQUAL(3, write_result.value());

  read_result = buffer.read(read_data, 2);
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(2, read_result.value());
  TEST_ASSERT_EQUAL(1, buffer.size());
  TEST_ASSERT_EQUAL(1, read_data[0]);
  TEST_ASSERT_EQUAL(2, read_data[1]);

  // Test read more than buffer size
  read_result = buffer.read(read_data, 3);
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(1, read_result.value());
  TEST_ASSERT_EQUAL(0, buffer.size());
  TEST_ASSERT_EQUAL(3, read_data[0]);
}

void test_FixedLengthLinearBuffer_read_null() {
  malib::FixedLengthLinearBuffer<int, 4> buffer;
  int data[] = {1, 2, 3};
  buffer.write(data, 3);

  auto result = buffer.read(nullptr, 2);
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::NullPointerInput, result.error());
}

void test_FixedLengthLinearBuffer_read_non_trivial() {
  malib::FixedLengthLinearBuffer<std::string, 3> buffer;
  std::string write_data[] = {"test1", "test2", "test3"};
  buffer.write(write_data, 3);

  std::string read_data[3];
  auto result = buffer.read(read_data, 3);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());
  TEST_ASSERT_EQUAL_STRING("test1", read_data[0].c_str());
  TEST_ASSERT_EQUAL_STRING("test2", read_data[1].c_str());
  TEST_ASSERT_EQUAL_STRING("test3", read_data[2].c_str());
}

void test_FixedLengthLinearBuffer_read_non_trivial_partial() {
  malib::FixedLengthLinearBuffer<std::string, 3> buffer;
  std::string write_data[] = {"first", "second", "third"};
  buffer.write(write_data, 3);

  // Read first two strings
  std::string read_data[2];
  auto result = buffer.read(read_data, 2);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(2, result.value());
  TEST_ASSERT_EQUAL(1, buffer.size());
  TEST_ASSERT_EQUAL_STRING("first", read_data[0].c_str());
  TEST_ASSERT_EQUAL_STRING("second", read_data[1].c_str());

  // Read remaining string
  std::string remaining_data[1];
  result = buffer.read(remaining_data, 1);
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());
  TEST_ASSERT_EQUAL(0, buffer.size());
  TEST_ASSERT_EQUAL_STRING("third", remaining_data[0].c_str());
}

void test_FixedLengthLinearBuffer() {
  RUN_TEST(test_FixedLengthLinearBuffer_trivially_copyable_type);
  RUN_TEST(test_FixedLengthLinearBuffer_non_trivially_copyable_type);
  RUN_TEST(test_FixedLengthLinearBuffer_zero_size_write);
  RUN_TEST(test_FixedLengthLinearBuffer_edge_cases);
  RUN_TEST(test_FixedLengthLinearBuffer_write_partial);
  RUN_TEST(test_FixedLengthLinearBuffer_write_non_trivial);
  RUN_TEST(test_FixedLengthLinearBuffer_append_one);
  RUN_TEST(test_FixedLengthLinearBuffer_read_write);
  RUN_TEST(test_FixedLengthLinearBuffer_read_null);
  RUN_TEST(test_FixedLengthLinearBuffer_read_non_trivial);
  RUN_TEST(test_FixedLengthLinearBuffer_read_non_trivial_partial);
}