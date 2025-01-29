#include <unity.h>

extern void test_fixed_string_buffer_copy();
extern void test_fixed_string_buffer_reset();

extern void test_token_view();

extern void test_tokenizer_tokenize();
extern void test_tokenizer_element_access();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fixed_string_buffer_copy);
  RUN_TEST(test_fixed_string_buffer_reset);
  RUN_TEST(test_token_view);
  RUN_TEST(test_tokenizer_tokenize);
  RUN_TEST(test_tokenizer_element_access);
  return UNITY_END();
}
