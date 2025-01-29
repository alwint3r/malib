#include <unity.h>

extern void test_FixedStringBuffer();
extern void test_Token();
extern void test_Tokenizer();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();

  test_FixedStringBuffer();
  test_Token();
  test_Tokenizer();

  return UNITY_END();
}
