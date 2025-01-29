#include <unity.h>

extern void test_fixed_string_buffer_copy();
extern void test_fixed_string_buffer_reset();

void setUp() {}

void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fixed_string_buffer_copy);
  RUN_TEST(test_fixed_string_buffer_reset);

  return UNITY_END();
}
