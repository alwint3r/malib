#include <unity.h>

#include <Token.hpp>

void test_token_view() {
  // null pointer test
  malib::Token tok1{nullptr, 0, 0};
  auto result1 = tok1.view();
  TEST_ASSERT_FALSE(result1.has_value());
  TEST_ASSERT_EQUAL(malib::Token::Error::NullPointerData, result1.error());

  // invalid size test
  const char* data = "hello";
  malib::Token tok2{data, 1, 0};
  auto result2 = tok2.view();
  TEST_ASSERT_FALSE(result2.has_value());
  TEST_ASSERT_EQUAL(malib::Token::Error::InvalidSize, result2.error());
}