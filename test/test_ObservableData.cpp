#include <unity.h>

#include "malib/ObservableData.hpp"

void test_no_subscribers() {
  malib::ObservableData<int, 0> data;
  data.update(42);
  TEST_ASSERT_EQUAL(42, data.get());
}

void test_single_subscriber() {
  malib::ObservableData<int, 1> data{};
  bool notified = false;
  data.subscribe([&notified](auto&) { notified = true; });
  data.update(42);
  TEST_ASSERT_TRUE(notified);
  TEST_ASSERT_EQUAL(42, data.get());
}

void test_multiple_subscribers() {
  malib::ObservableData<int, 2> data;
  bool notified1 = false;
  bool notified2 = false;
  data.subscribe([&notified1](auto&) { notified1 = true; });
  data.subscribe([&notified2](auto&) { notified2 = true; });
  data.update(42);
  TEST_ASSERT_TRUE(notified1);
  TEST_ASSERT_TRUE(notified2);
  TEST_ASSERT_EQUAL(42, data.get());
}

void test_ObservableData() {
  RUN_TEST(test_no_subscribers);
  RUN_TEST(test_single_subscriber);
  RUN_TEST(test_multiple_subscribers);
}