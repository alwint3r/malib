#include <unity.h>
#include <malib/StringConverter.hpp>

void test_StringConverter_ToInt16() {
  std::string_view input = "123";
  int16_t i16 = 0;
  auto result = malib::StringConverter::to_number(input, i16);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_INT16(123, i16);
}

void test_StringConverter_ToInt32() {
  std::string_view input = "123456";
  int32_t i32 = 0;
  auto result = malib::StringConverter::to_number(input, i32);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_INT32(123456, i32);
}

void test_StringConverter_ToFloat() {
  std::string_view input = "123.45";
  float f = 0.0f;
  auto result = malib::StringConverter::to_number(input, f);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 123.45, f);
}

void test_StringConverter_InvalidArgument() {
  std::string_view input = "abc";
  int16_t i16 = 0;
  auto result = malib::StringConverter::to_number(input, i16);
  TEST_ASSERT_EQUAL(malib::Error::InvalidArgument, result);
}

void test_StringConverter_ResultOutOfRange() {
  std::string_view input = "32768";  // Out of range for int16_t
  int16_t i16 = 0;
  auto result = malib::StringConverter::to_number(input, i16);
  TEST_ASSERT_EQUAL(malib::Error::ResultOutOfRange, result);
}

void test_StringConverter() {
  RUN_TEST(test_StringConverter_ToInt16);
  RUN_TEST(test_StringConverter_ToInt32);
  RUN_TEST(test_StringConverter_ToFloat);
  RUN_TEST(test_StringConverter_InvalidArgument);
  RUN_TEST(test_StringConverter_ResultOutOfRange);
}
