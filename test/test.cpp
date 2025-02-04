#include <unity.h>

extern void test_FixedStringBuffer();
extern void test_Token();
extern void test_Tokenizer();
extern void test_RingBuffer();
extern void test_BufferReader();
extern void test_Shell();
extern void test_WaitableQueue();
extern void test_TokenReader();
extern void test_ObservableData();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();

  test_FixedStringBuffer();
  test_Token();
  test_Tokenizer();
  test_RingBuffer();
  test_BufferReader();
  test_Shell();
  test_WaitableQueue();
  test_TokenReader();
  test_ObservableData();

  return UNITY_END();
}
