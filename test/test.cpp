#include <unity.h>

extern void test_fixed_string_buffer_copy();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fixed_string_buffer_copy);

  return UNITY_END();
}
