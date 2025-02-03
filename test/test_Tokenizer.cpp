#include <unity.h>

#include "malib/Error.hpp"
#include "malib/Tokenizer.hpp"

void test_tokenizer_tokenize_ls_al() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};
  auto result = tokenizer.tokenize("ls -al");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());
}

void test_tokenizer_tokenize_ls_al_h() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};
  auto result = tokenizer.tokenize("ls -al -h");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(3, result.value());
}

void test_tokenizer_tokenize_exceed_max_tokens() {
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};
  auto result = tinyTokenizer.tokenize("ls -al -h");
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, result.error());
}

void test_tokenizer_tokenize_ls() {
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};
  auto result = tinyTokenizer.tokenize("ls");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(1, result.value());
}

void test_tokenizer_tokenize_multiple_spaces() {
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};
  auto result = tinyTokenizer.tokenize("ls      -al");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());
}

void test_tokenizer_tokenize_quoted_string() {
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};
  auto result = tinyTokenizer.tokenize("echo \"hello world\"");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());
}

void test_tokenizer_tokenize_empty_string() {
  using TinyTokenizer = malib::Tokenizer<2>;
  TinyTokenizer tinyTokenizer{};
  auto result = tinyTokenizer.tokenize("");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(0, result.value());
}

void test_tokenizer_element_access_ls_al() {
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
}

void test_tokenizer_element_access_out_of_bounds() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};
  auto result = tokenizer.tokenize("ls -al");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(2, result.value());

  auto token = tokenizer[2];
  TEST_ASSERT_FALSE(token.has_value());
  TEST_ASSERT_EQUAL(malib::Error::IndexOutOfRange, token.error());
}

void test_tokenizer_tokens_vector() {
  using SmallTokenizer = malib::Tokenizer<10>;
  SmallTokenizer tokenizer{};
  auto result = tokenizer.tokenize("ls -al -h");
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL_INT(3, result.value());

  auto tokens = tokenizer.tokens_vector();
  TEST_ASSERT_EQUAL_INT(3, tokens.size());

  TEST_ASSERT_EQUAL(tokens[0].start, 0);
  TEST_ASSERT_EQUAL(tokens[0].end, 1);
  auto view = tokens[0].view().value();
  TEST_ASSERT_EQUAL_STRING_LEN("ls", view.data(), view.size());

  TEST_ASSERT_EQUAL(tokens[1].start, 3);
  TEST_ASSERT_EQUAL(tokens[1].end, 5);
  view = tokens[1].view().value();
  TEST_ASSERT_EQUAL_STRING_LEN("-al", view.data(), view.size());

  TEST_ASSERT_EQUAL(tokens[2].start, 7);
  TEST_ASSERT_EQUAL(tokens[2].end, 8);
  view = tokens[2].view().value();
  TEST_ASSERT_EQUAL_STRING_LEN("-h", view.data(), view.size());
}

void test_Tokenizer() {
  RUN_TEST(test_tokenizer_tokenize_ls_al);
  RUN_TEST(test_tokenizer_tokenize_ls_al_h);
  RUN_TEST(test_tokenizer_tokenize_exceed_max_tokens);
  RUN_TEST(test_tokenizer_tokenize_ls);
  RUN_TEST(test_tokenizer_tokenize_multiple_spaces);
  RUN_TEST(test_tokenizer_tokenize_quoted_string);
  RUN_TEST(test_tokenizer_tokenize_empty_string);
  RUN_TEST(test_tokenizer_element_access_ls_al);
  RUN_TEST(test_tokenizer_element_access_out_of_bounds);
  RUN_TEST(test_tokenizer_tokens_vector);
}