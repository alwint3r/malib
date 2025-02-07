#include <unity.h>

#include <malib/Error.hpp>
#include <malib/Token.hpp>

void test_token_empty() {
  malib::Token tok{0, 0};
  TEST_ASSERT_TRUE(tok.empty());
}

void test_token_view_valid_case() {
  std::string_view base = "hello";
  malib::Token tok{1, 3};
  auto view = tok.view(base);
  TEST_ASSERT_EQUAL_STRING_LEN("ell", view.data(), 3);
}

void test_token_view_edge_case_single_char() {
  std::string_view base = "hello";
  malib::Token tok{2, 1};
  auto view = tok.view(base);
  TEST_ASSERT_EQUAL_STRING_LEN("l", view.data(), 1);
}

void test_token_view_edge_case_full_string() {
  std::string_view base = "hello";
  malib::Token tok{0, 5};
  auto view = tok.view(base);
  TEST_ASSERT_EQUAL_STRING_LEN("hello", view.data(), 5);
}

void test_token_default_initialize() {
  malib::Token tok{};
  TEST_ASSERT_EQUAL(0, tok.offset);
  TEST_ASSERT_EQUAL(0, tok.length);
  TEST_ASSERT_TRUE(tok.empty());
}

void test_Token() {
  RUN_TEST(test_token_empty);
  RUN_TEST(test_token_view_valid_case);
  RUN_TEST(test_token_view_edge_case_single_char);
  RUN_TEST(test_token_view_edge_case_full_string);
  RUN_TEST(test_token_default_initialize);
}