#include <unity.h>

#include <Token.hpp>

void test_token_view_nullptr() {
  malib::Token tok1{nullptr, 0, 0};
  auto result1 = tok1.view();
  TEST_ASSERT_FALSE(result1.has_value());
  TEST_ASSERT_EQUAL(malib::Token::Error::NullPointerData, result1.error());
}

void test_token_view_invalid_size() {
  const char* data = "hello";
  malib::Token tok2{data, 1, 0};
  auto result2 = tok2.view();
  TEST_ASSERT_FALSE(result2.has_value());
  TEST_ASSERT_EQUAL(malib::Token::Error::InvalidSize, result2.error());
}

void test_token_view_valid_case() {
  const char* data = "hello";
  malib::Token tok3{data, 1, 3};
  auto result3 = tok3.view();
  TEST_ASSERT_TRUE(result3.has_value());
  TEST_ASSERT_EQUAL_STRING_LEN("ell", result3->data(), 3);
}

void test_token_view_edge_case_same_start_end() {
  const char* data = "hello";
  malib::Token tok4{data, 2, 2};
  auto result4 = tok4.view();
  TEST_ASSERT_TRUE(result4.has_value());
  TEST_ASSERT_EQUAL_STRING_LEN("l", result4->data(), 1);
}

void test_token_view_edge_case_full_string() {
  const char* data = "hello";
  malib::Token tok5{data, 0, 4};
  auto result5 = tok5.view();
  TEST_ASSERT_TRUE(result5.has_value());
  TEST_ASSERT_EQUAL_STRING_LEN("hello", result5->data(), 5);
}

void test_Token() {
  RUN_TEST(test_token_view_nullptr);
  RUN_TEST(test_token_view_invalid_size);
  RUN_TEST(test_token_view_valid_case);
  RUN_TEST(test_token_view_edge_case_same_start_end);
  RUN_TEST(test_token_view_edge_case_full_string);
}