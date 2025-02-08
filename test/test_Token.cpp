#include <unity.h>

#include <malib/Error.hpp>
#include <malib/Token.hpp>
#include <vector>

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

void test_tokenviews_empty() {
  std::string_view base = "";
  std::vector<malib::Token> tokens;
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_TRUE(views.has_value());
}

void test_tokenviews_iteration() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{0, 5}, {6, 5}};
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_TRUE(views.has_value());

  auto it = views->begin();
  auto view = *it;
  TEST_ASSERT_EQUAL_STRING_LEN("hello", view.data(), 5);
  ++it;
  view = *it;
  TEST_ASSERT_EQUAL_STRING_LEN("world", view.data(), 5);
  ++it;
  TEST_ASSERT_TRUE(it == views->end());
}

void test_tokenviews_indexing() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{0, 5}, {6, 5}};
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_TRUE(views.has_value());

  auto result0 = (*views)[0];
  TEST_ASSERT_TRUE(result0.has_value());
  TEST_ASSERT_EQUAL_STRING_LEN("hello", result0->data(), 5);

  auto result1 = (*views)[1];
  TEST_ASSERT_TRUE(result1.has_value());
  TEST_ASSERT_EQUAL_STRING_LEN("world", result1->data(), 5);

  auto invalid = (*views)[2];
  TEST_ASSERT_FALSE(invalid.has_value());
  TEST_ASSERT_EQUAL(malib::Error::IndexOutOfRange, invalid.error());
}

void test_tokenviews_size() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{0, 5}, {6, 5}};
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_TRUE(views.has_value());
  TEST_ASSERT_EQUAL(2, views->size());
  TEST_ASSERT_FALSE(views->empty());
}

void test_tokenviews_invalid_token() {
  std::string_view base = "hello";
  std::vector<malib::Token> tokens = {{0, 6}};  // length exceeds base string
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_FALSE(views.has_value());
  TEST_ASSERT_EQUAL(malib::Error::IndexOutOfRange, views.error());
}

void test_tokenviews_overlapping_tokens() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{0, 5}, {4, 5}};  // overlapping tokens
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_FALSE(views.has_value());
  TEST_ASSERT_EQUAL(malib::Error::InvalidArgument, views.error());
}

void test_tokenviews_unordered_tokens() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{6, 5}, {0, 5}};  // out of order
  auto views = malib::TokenViews::create(base, tokens);
  TEST_ASSERT_FALSE(views.has_value());
  TEST_ASSERT_EQUAL(malib::Error::InvalidArgument, views.error());
}

void test_tokenviews_subspan() {
  std::string_view base = "hello world";
  std::vector<malib::Token> tokens = {{0, 5}, {6, 5}, {0, 11}};
  auto views = malib::TokenViews::create(
      base, std::span(tokens.data(), 2));  // only first two tokens
  TEST_ASSERT_TRUE(views.has_value());
  TEST_ASSERT_EQUAL(2, views->size());
}

void test_TokenViews() {
  RUN_TEST(test_tokenviews_empty);
  RUN_TEST(test_tokenviews_iteration);
  RUN_TEST(test_tokenviews_indexing);
  RUN_TEST(test_tokenviews_size);
  RUN_TEST(test_tokenviews_invalid_token);
  RUN_TEST(test_tokenviews_overlapping_tokens);
  RUN_TEST(test_tokenviews_unordered_tokens);
  RUN_TEST(test_tokenviews_subspan);
}

void test_Token() {
  RUN_TEST(test_token_empty);
  RUN_TEST(test_token_view_valid_case);
  RUN_TEST(test_token_view_edge_case_single_char);
  RUN_TEST(test_token_view_edge_case_full_string);
  RUN_TEST(test_token_default_initialize);
  test_TokenViews();
}