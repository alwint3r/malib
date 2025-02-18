#include <unity.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <malib/FixedLengthLinearBuffer.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

void test_FixedLengthLinearBuffer_aligned_structs() {
  // 4-byte aligned struct
  struct alignas(4) Aligned4 {
    char a;
    int32_t b;  // This will have 3 padding bytes after 'a'
  };

  // 8-byte aligned struct
  struct alignas(8) Aligned8 {
    char a;
    int64_t b;  // This will have 7 padding bytes after 'a'
  };

  // Test 4-byte aligned struct
  {
    malib::FixedLengthLinearBuffer<Aligned4, 3> buffer;
    Aligned4 data[2] = {{1, 100}, {2, 200}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    Aligned4 read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(1, read_data[0].a);
    TEST_ASSERT_EQUAL(100, read_data[0].b);
    TEST_ASSERT_EQUAL(2, read_data[1].a);
    TEST_ASSERT_EQUAL(200, read_data[1].b);
  }

  // Test 8-byte aligned struct
  {
    malib::FixedLengthLinearBuffer<Aligned8, 3> buffer;
    Aligned8 data[2] = {{1, 1000000}, {2, 2000000}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    Aligned8 read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(1, read_data[0].a);
    TEST_ASSERT_EQUAL(1000000, read_data[0].b);
    TEST_ASSERT_EQUAL(2, read_data[1].a);
    TEST_ASSERT_EQUAL(2000000, read_data[1].b);
  }
}

void test_FixedLengthLinearBuffer_packed_structs() {
#pragma pack(push, 1)
  struct PackedStruct {
    char a;     // 1 byte
    int32_t b;  // 4 bytes, no padding before
    char c;     // 1 byte
  };  // total: 6 bytes

  struct LargePackedStruct {
    char a;     // 1 byte
    int64_t b;  // 8 bytes, no padding before
    int16_t c;  // 2 bytes, no padding before
    char d;     // 1 byte
  };  // total: 12 bytes
#pragma pack(pop)

  // Verify the structs are actually packed
  static_assert(sizeof(PackedStruct) == 6,
                "PackedStruct is not properly packed");
  static_assert(sizeof(LargePackedStruct) == 12,
                "LargePackedStruct is not properly packed");

  // Test small packed struct
  {
    malib::FixedLengthLinearBuffer<PackedStruct, 3> buffer;
    PackedStruct data[2] = {{1, 100, 2}, {3, 200, 4}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    PackedStruct read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(1, read_data[0].a);
    TEST_ASSERT_EQUAL(100, read_data[0].b);
    TEST_ASSERT_EQUAL(2, read_data[0].c);
    TEST_ASSERT_EQUAL(3, read_data[1].a);
    TEST_ASSERT_EQUAL(200, read_data[1].b);
    TEST_ASSERT_EQUAL(4, read_data[1].c);
  }

  // Test large packed struct
  {
    malib::FixedLengthLinearBuffer<LargePackedStruct, 3> buffer;
    LargePackedStruct data[2] = {{1, 1000000, 42, 3}, {4, 2000000, 84, 6}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    LargePackedStruct read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(1, read_data[0].a);
    TEST_ASSERT_EQUAL(1000000, read_data[0].b);
    TEST_ASSERT_EQUAL(42, read_data[0].c);
    TEST_ASSERT_EQUAL(3, read_data[0].d);
    TEST_ASSERT_EQUAL(4, read_data[1].a);
    TEST_ASSERT_EQUAL(2000000, read_data[1].b);
    TEST_ASSERT_EQUAL(84, read_data[1].c);
    TEST_ASSERT_EQUAL(6, read_data[1].d);
  }
}

void test_FixedLengthLinearBuffer_bitfields() {
  struct BitFieldStruct {
    unsigned int a : 3;  // 3 bits
    unsigned int b : 5;  // 5 bits
    unsigned int c : 4;  // 4 bits
    int d : 20;          // 20 bits
  };  // total: 32 bits (4 bytes)

  struct ComplexBitFields {
    unsigned int flag1 : 1;    // 1 bit
    unsigned int value1 : 7;   // 7 bits
    unsigned int : 0;          // Force alignment to next word boundary
    unsigned int flag2 : 1;    // 1 bit
    unsigned int value2 : 15;  // 15 bits
    signed int svalue : 8;     // 8 bits
  };

  // Test simple bitfields
  {
    malib::FixedLengthLinearBuffer<BitFieldStruct, 3> buffer;
    BitFieldStruct data[2] = {
        {/* a */ 5, /* b */ 16, /* c */ 10, /* d */ -1024},
        {/* a */ 2, /* b */ 30, /* c */ 12, /* d */ 1024}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    BitFieldStruct read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(5, read_data[0].a);
    TEST_ASSERT_EQUAL(16, read_data[0].b);
    TEST_ASSERT_EQUAL(10, read_data[0].c);
    TEST_ASSERT_EQUAL(-1024, read_data[0].d);
    TEST_ASSERT_EQUAL(2, read_data[1].a);
    TEST_ASSERT_EQUAL(30, read_data[1].b);
    TEST_ASSERT_EQUAL(12, read_data[1].c);
    TEST_ASSERT_EQUAL(1024, read_data[1].d);
  }

  // Test complex bitfields with alignment and mixed signs
  {
    malib::FixedLengthLinearBuffer<ComplexBitFields, 3> buffer;
    ComplexBitFields data[2] = {{/* flag1 */ 1, /* value1 */ 127, /* flag2 */ 0,
                                 /* value2 */ 1024, /* svalue */ -42},
                                {/* flag1 */ 0, /* value1 */ 64, /* flag2 */ 1,
                                 /* value2 */ 32000, /* svalue */ 42}};

    auto write_result = buffer.write(data, 2);
    TEST_ASSERT_TRUE(write_result.has_value());
    TEST_ASSERT_EQUAL(2, write_result.value());

    ComplexBitFields read_data[2];
    auto read_result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(2, read_result.value());

    TEST_ASSERT_EQUAL(1, read_data[0].flag1);
    TEST_ASSERT_EQUAL(127, read_data[0].value1);
    TEST_ASSERT_EQUAL(0, read_data[0].flag2);
    TEST_ASSERT_EQUAL(1024, read_data[0].value2);
    TEST_ASSERT_EQUAL(-42, read_data[0].svalue);

    TEST_ASSERT_EQUAL(0, read_data[1].flag1);
    TEST_ASSERT_EQUAL(64, read_data[1].value1);
    TEST_ASSERT_EQUAL(1, read_data[1].flag2);
    TEST_ASSERT_EQUAL(32000, read_data[1].value2);
    TEST_ASSERT_EQUAL(42, read_data[1].svalue);
  }
}

void test_FixedLengthLinearBuffer_iterators() {
  malib::FixedLengthLinearBuffer<int, 4> buffer;

  // Test empty buffer iterators
  TEST_ASSERT_TRUE(buffer.begin() == buffer.end());
  TEST_ASSERT_TRUE(buffer.cbegin() == buffer.cend());
  TEST_ASSERT_TRUE(buffer.rbegin() == buffer.rend());
  TEST_ASSERT_TRUE(buffer.crbegin() == buffer.crend());

  // Fill buffer with data
  int data[] = {1, 2, 3, 4};
  buffer.write(data, 4);

  // Test non-const forward iteration and modification
  {
    int expected = 1;
    for (auto& value : buffer) {
      TEST_ASSERT_EQUAL(expected++, value);
      value *= 2;  // Modify through iterator
    }
  }

  // Test const forward iteration with modified values
  {
    int expected[] = {2, 4, 6, 8};
    int i = 0;
    const auto& const_buffer = buffer;
    for (const auto& value : const_buffer) {
      TEST_ASSERT_EQUAL(expected[i++], value);
    }
  }

  // Test reverse iteration with modified values
  {
    int expected[] = {8, 6, 4, 2};
    int i = 0;
    for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
      TEST_ASSERT_EQUAL(expected[i++], *it);
    }
  }

  // Test const reverse iteration with modified values
  {
    int expected[] = {8, 6, 4, 2};
    int i = 0;
    const auto& const_buffer = buffer;
    for (auto it = const_buffer.crbegin(); it != const_buffer.crend(); ++it) {
      TEST_ASSERT_EQUAL(expected[i++], *it);
    }
  }

  // Test iterator arithmetic and comparison
  {
    auto it = buffer.begin();
    TEST_ASSERT_EQUAL(4, *(it + 1));  // Second element is now 4 (2*2)
    TEST_ASSERT_EQUAL(6, *(it + 2));  // Third element is now 6 (3*2)
    TEST_ASSERT_TRUE(it < buffer.end());
    TEST_ASSERT_TRUE(it + 4 == buffer.end());
  }
}

void test_FixedLengthLinearBuffer_iterators_string() {
  malib::FixedLengthLinearBuffer<std::string, 4> buffer;

  // Test empty buffer iterators
  TEST_ASSERT_TRUE(buffer.begin() == buffer.end());
  TEST_ASSERT_TRUE(buffer.cbegin() == buffer.cend());
  TEST_ASSERT_TRUE(buffer.rbegin() == buffer.rend());
  TEST_ASSERT_TRUE(buffer.crbegin() == buffer.crend());

  // Fill buffer with data
  std::string data[] = {"one", "two", "three", "four"};
  buffer.write(data, 4);

  // Test non-const forward iteration and modification
  {
    std::string expected[] = {"one", "two", "three", "four"};
    int i = 0;
    for (auto& value : buffer) {
      TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), value.c_str());
      value += "_mod";  // Modify through iterator
      i++;
    }
  }

  // Test const forward iteration with modified values
  {
    std::string expected[] = {"one_mod", "two_mod", "three_mod", "four_mod"};
    int i = 0;
    const auto& const_buffer = buffer;
    for (const auto& value : const_buffer) {
      TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), value.c_str());
      i++;
    }
  }

  // Test reverse iteration with modified values
  {
    std::string expected[] = {"four_mod", "three_mod", "two_mod", "one_mod"};
    int i = 0;
    for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
      TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), it->c_str());
      i++;
    }
  }

  // Test iterator arithmetic and comparison
  {
    auto it = buffer.begin();
    TEST_ASSERT_EQUAL_STRING("two_mod", (*(it + 1)).c_str());
    TEST_ASSERT_EQUAL_STRING("three_mod", (*(it + 2)).c_str());
    TEST_ASSERT_TRUE(it < buffer.end());
    TEST_ASSERT_TRUE(it + 4 == buffer.end());
  }
}

void test_FixedLengthLinearBuffer_format_char() {
  malib::FixedLengthLinearBuffer<char, 32> buffer;

  // Write formatted string into buffer
  std::string formatted = std::format("Test {:d}, {:.2f}", 42, 3.14159);
  auto result = buffer.write(formatted.data(), formatted.size());
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(formatted.size(), result.value());

  // Verify the content
  char read_data[32] = {0};
  auto read_result = buffer.read(read_data, formatted.size());
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(formatted.size(), read_result.value());
  TEST_ASSERT_EQUAL_STRING("Test 42, 3.14", read_data);

  // Test writing multiple formatted strings
  buffer.clear();
  std::string str1 = std::format("{:>5}", "abc");
  std::string str2 = std::format("{:<5}", "xyz");

  result = buffer.write(str1.data(), str1.size());
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(str1.size(), result.value());

  result = buffer.write(str2.data(), str2.size());
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(str2.size(), result.value());

  // Read and verify combined result
  std::memset(read_data, 0, sizeof(read_data));
  read_result = buffer.read(read_data, str1.size() + str2.size());
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(str1.size() + str2.size(), read_result.value());
  TEST_ASSERT_EQUAL_STRING("  abcxyz  ", read_data);
}

void test_FixedLengthLinearBuffer_format_to() {
  malib::FixedLengthLinearBuffer<char, 32> buffer;

  // Format directly into buffer using format_to with sizing_iterator
  auto out =
      std::format_to(buffer.format_begin(), "Test {:d}, {:.2f}", 42, 3.14159);
  TEST_ASSERT_FALSE(buffer.empty());
  auto chars_written = buffer.size();
  TEST_ASSERT_GREATER_THAN(0, chars_written);

  // Verify the content
  char read_data[32] = {0};
  auto read_result = buffer.read(read_data, chars_written);
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(chars_written, read_result.value());
  TEST_ASSERT_EQUAL_STRING("Test 42, 3.14", read_data);

  // Test multiple format_to operations
  buffer.clear();
  auto it = buffer.format_begin();
  it = std::format_to(it, "{:>5}", "abc");
  it = std::format_to(it, "{:<5}", "xyz");

  TEST_ASSERT_GREATER_THAN(0, buffer.size());

  // Read and verify combined result
  std::memset(read_data, 0, sizeof(read_data));
  read_result = buffer.read(read_data, buffer.size());
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL_STRING("  abcxyz  ", read_data);
}

void test_FixedLengthLinearBuffer_format_to_boundary() {
  malib::FixedLengthLinearBuffer<char, 10> buffer;

  // Try to format a string that would exceed buffer capacity
  auto it = buffer.format_begin();
  it = std::format_to(it, "{:>12}",
                      "test");  // Should truncate at buffer boundary

  TEST_ASSERT_EQUAL(10,
                    buffer.size());  // Buffer should be full but not overflown
  TEST_ASSERT_TRUE(buffer.full());

  // Verify the content
  char read_data[11] = {0};  // +1 for null terminator
  auto read_result = buffer.read(read_data, buffer.size());
  TEST_ASSERT_TRUE(read_result.has_value());
  TEST_ASSERT_EQUAL(10, read_result.value());
  TEST_ASSERT_EQUAL(10, strlen(read_data));  // Ensure we got exactly 10 chars

  // Try multiple format operations until buffer is full
  buffer.clear();
  it = buffer.format_begin();
  it = std::format_to(it, "{:5}", "abc");    // Takes 5 chars
  it = std::format_to(it, "{:>6}", "test");  // Would need 6 chars, gets only 5

  TEST_ASSERT_EQUAL(10, buffer.size());
  TEST_ASSERT_TRUE(buffer.full());
}

void test_FixedLengthLinearBuffer_string_view() {
  // Test with char buffer
  {
    malib::FixedLengthLinearBuffer<char, 32> buffer;

    // Test empty buffer
    auto view = buffer.as_string_view();
    TEST_ASSERT_EQUAL(0, view.size());
    TEST_ASSERT_TRUE(view.empty());

    // Test with content
    const char* test_str = "Hello, World!";
    auto result = buffer.write(test_str, strlen(test_str));
    TEST_ASSERT_TRUE(result.has_value());

    view = buffer.as_string_view();
    TEST_ASSERT_EQUAL(strlen(test_str), view.size());
    TEST_ASSERT_EQUAL_STRING(test_str, view.data());

    // Test after partial read - read first 7 chars ("Hello, ")
    char read_buf[8];
    auto read_result = buffer.read(read_buf, 7);
    TEST_ASSERT_TRUE(read_result.has_value());
    TEST_ASSERT_EQUAL(7, read_result.value());

    view = buffer.as_string_view();
    TEST_ASSERT_EQUAL(6, view.size());
    TEST_ASSERT_EQUAL_STRING_LEN("World!", view.data(), view.size());
  }

  // Test with wchar_t buffer
  {
    malib::FixedLengthLinearBuffer<wchar_t, 32> wbuffer;

    // Test empty buffer
    auto view = wbuffer.as_wstring_view();
    TEST_ASSERT_EQUAL(0, view.size());
    TEST_ASSERT_TRUE(view.empty());

    // Test with content
    const wchar_t test_str[] = L"Hello, World!";
    auto result = wbuffer.write(test_str, wcslen(test_str));
    TEST_ASSERT_TRUE(result.has_value());

    view = wbuffer.as_wstring_view();
    TEST_ASSERT_EQUAL(wcslen(test_str), view.size());
    TEST_ASSERT_EQUAL(0, wmemcmp(test_str, view.data(), view.size()));
  }
}

void test_FixedLengthLinearBuffer_reset_on_read() {
  // Test with trivially copyable type
  {
    malib::FixedLengthLinearBuffer<int, 4, false, true> buffer;
    int data[] = {1, 2, 3, 4};
    buffer.write(data, 4);

    // Read first two elements
    int read_data[2];
    auto result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL(2, result.value());
    TEST_ASSERT_EQUAL(1, read_data[0]);
    TEST_ASSERT_EQUAL(2, read_data[1]);

    // Check remaining elements and verify reset area is zeroed
    TEST_ASSERT_EQUAL(2, buffer.size());
    auto* raw_buffer = buffer.data();
    TEST_ASSERT_EQUAL(3, raw_buffer[0]);  // First remaining element
    TEST_ASSERT_EQUAL(4, raw_buffer[1]);  // Second remaining element
    TEST_ASSERT_EQUAL(0, raw_buffer[2]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[3]);  // Reset area
  }

  // Test with non-trivially copyable type
  {
    malib::FixedLengthLinearBuffer<std::string, 4, false, true> buffer;
    std::string data[] = {"one", "two", "three", "four"};
    buffer.write(data, 4);

    // Read first two elements
    std::string read_data[2];
    auto result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL(2, result.value());
    TEST_ASSERT_EQUAL_STRING("one", read_data[0].c_str());
    TEST_ASSERT_EQUAL_STRING("two", read_data[1].c_str());

    // Check remaining elements and verify reset area is empty strings
    TEST_ASSERT_EQUAL(2, buffer.size());
    auto* raw_buffer = buffer.data();
    TEST_ASSERT_EQUAL_STRING("three",
                             raw_buffer[0].c_str());  // First remaining element
    TEST_ASSERT_EQUAL_STRING(
        "four", raw_buffer[1].c_str());       // Second remaining element
    TEST_ASSERT_TRUE(raw_buffer[2].empty());  // Reset area
    TEST_ASSERT_TRUE(raw_buffer[3].empty());  // Reset area
  }

  // Test with partial reads
  {
    malib::FixedLengthLinearBuffer<int, 6, false, true> buffer;
    int data[] = {1, 2, 3, 4, 5, 6};
    buffer.write(data, 6);

    // Read in multiple steps
    int read_data[2];

    // First read (2 elements)
    auto result = buffer.read(read_data, 2);
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL(2, result.value());
    TEST_ASSERT_EQUAL(1, read_data[0]);
    TEST_ASSERT_EQUAL(2, read_data[1]);
    TEST_ASSERT_EQUAL(4, buffer.size());

    // Check buffer state after first read
    auto* raw_buffer = buffer.data();
    TEST_ASSERT_EQUAL(3, raw_buffer[0]);
    TEST_ASSERT_EQUAL(4, raw_buffer[1]);
    TEST_ASSERT_EQUAL(5, raw_buffer[2]);
    TEST_ASSERT_EQUAL(6, raw_buffer[3]);
    TEST_ASSERT_EQUAL(0, raw_buffer[4]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[5]);  // Reset area

    // Second read (3 elements)
    int read_data2[3];
    result = buffer.read(read_data2, 3);
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL(3, result.value());
    TEST_ASSERT_EQUAL(3, read_data2[0]);
    TEST_ASSERT_EQUAL(4, read_data2[1]);
    TEST_ASSERT_EQUAL(5, read_data2[2]);
    TEST_ASSERT_EQUAL(1, buffer.size());

    // Check buffer state after second read
    TEST_ASSERT_EQUAL(6, raw_buffer[0]);
    TEST_ASSERT_EQUAL(0, raw_buffer[1]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[2]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[3]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[4]);  // Reset area
    TEST_ASSERT_EQUAL(0, raw_buffer[5]);  // Reset area
  }
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
  RUN_TEST(test_FixedLengthLinearBuffer_aligned_structs);
  RUN_TEST(test_FixedLengthLinearBuffer_packed_structs);
  RUN_TEST(test_FixedLengthLinearBuffer_bitfields);
  RUN_TEST(test_FixedLengthLinearBuffer_iterators);
  RUN_TEST(test_FixedLengthLinearBuffer_iterators_string);
  RUN_TEST(test_FixedLengthLinearBuffer_format_char);
  RUN_TEST(test_FixedLengthLinearBuffer_format_to);
  RUN_TEST(test_FixedLengthLinearBuffer_format_to_boundary);
  RUN_TEST(test_FixedLengthLinearBuffer_string_view);
  RUN_TEST(test_FixedLengthLinearBuffer_reset_on_read);
}