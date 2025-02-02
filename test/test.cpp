#include <unity.h>

extern void test_FixedStringBuffer();
extern void test_Token();
extern void test_Tokenizer();
extern void test_RingBuffer();
extern void test_BufferReader();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();

  test_FixedStringBuffer();
  test_Token();
  test_Tokenizer();
  test_RingBuffer();
  test_BufferReader();

  return UNITY_END();
}
