#include <unity.h>

#include <malib/Token.hpp>
#include <malib/TokenReader.hpp>

void test_TokenReader_ToInt16() {
  malib::Token token{"123", 0, 2};
  int16_t i16 = 0;
  auto result = malib::TokenReader::to_number(token, i16);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_INT16(123, i16);
}

void test_TokenReader_ToInt32() {
  malib::Token token{"123456", 0, 6};
  int32_t i32 = 0;
  auto result = malib::TokenReader::to_number(token, i32);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_INT32(123456, i32);
}

void test_TokenReader_ToFloat() {
  malib::Token token{"123.45", 0, 6};
  float f = 0.0f;
  auto result = malib::TokenReader::to_number(token, f);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_FLOAT_WITHIN(0.001, 123.45, f);
}

void test_TokenReader_InvalidArgument() {
  malib::Token token{"abc", 0, 3};
  int16_t i16 = 0;
  auto result = malib::TokenReader::to_number(token, i16);
  TEST_ASSERT_EQUAL(malib::Error::InvalidArgument, result);
}

void test_TokenReader_ResultOutOfRange() {
  malib::Token token{"32768", 0, 5};  // Out of range for int16_t
  int16_t i16 = 0;
  auto result = malib::TokenReader::to_number(token, i16);
  TEST_ASSERT_EQUAL(malib::Error::ResultOutOfRange, result);
}

void test_TokenReader() {
  RUN_TEST(test_TokenReader_ToInt16);
  RUN_TEST(test_TokenReader_ToInt32);
  RUN_TEST(test_TokenReader_ToFloat);
  RUN_TEST(test_TokenReader_InvalidArgument);
  RUN_TEST(test_TokenReader_ResultOutOfRange);
}