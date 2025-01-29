#include <unity.h>

#include "Tokenizer.hpp"

void test_tokenizer_tokenize() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};

  auto result = tokenizer.tokenize("ls -al");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());

  result = tokenizer.tokenize("ls -al -h");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(3, result.value());

  // unhappy path
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};

  auto result2 = tinyTokenizer.tokenize("ls -al -h");
  TEST_ASSERT_FALSE(result2.has_value());
  TEST_ASSERT_EQUAL(TinyTokenizer::Error::MaximumTokensExceeded,
                    result2.error());

  result2 = tinyTokenizer.tokenize("ls");
  TEST_ASSERT_TRUE(result2.has_value());
  TEST_ASSERT_EQUAL_INT(1, result2.value());

  // test with multiple spaces
  result2 = tinyTokenizer.tokenize("ls      -al");
  TEST_ASSERT_TRUE(result2.has_value());
  TEST_ASSERT_EQUAL_INT(2, result2.value());

  // test with quoted string
  result2 = tinyTokenizer.tokenize("echo \"hello world\"");
  TEST_ASSERT_TRUE(result2.has_value());
  TEST_ASSERT_EQUAL_INT(2, result2.value());

  // test with empty string
  result2 = tinyTokenizer.tokenize("");
  TEST_ASSERT_TRUE(result2.has_value());
  TEST_ASSERT_EQUAL_INT(0, result2.value());
}

void test_tokenizer_element_access() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};
  auto result = tokenizer.tokenize("ls -al");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());

  auto token = tokenizer[0];
  TEST_ASSERT_TRUE(token.has_value());
  TEST_ASSERT_EQUAL(token.value().start, 0);
  TEST_ASSERT_EQUAL(token.value().end, 1);
  auto view = token.value().view().value();
  TEST_ASSERT_EQUAL_STRING_LEN("ls", view.data(), view.size());

  token = tokenizer[1];
  TEST_ASSERT_EQUAL(token.value().start, 3);
  TEST_ASSERT_EQUAL(token.value().end, 5);
  TEST_ASSERT_TRUE(token.has_value());
  view = token.value().view().value();
  TEST_ASSERT_EQUAL_STRING_LEN("-al", view.data(), view.size());

  // out of bounds access
  token = tokenizer[2];
  TEST_ASSERT_FALSE(token.has_value());
  TEST_ASSERT_EQUAL(SmallTokenizer::Error::TokenIndexOutOfRange, token.error());
}