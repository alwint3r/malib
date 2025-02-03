#include <unity.h>
#include <thread>

#include <malib/WaitableQueue.hpp>

void test_push() {
  malib::WaitableQueue<int> queue{};
  auto result = queue.push(1);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
}

void test_pop() {
  malib::WaitableQueue<int> queue{};
  queue.push(1);
  auto result = queue.pop();
  TEST_ASSERT_EQUAL(1, result);
}

void test_empty() {
  malib::WaitableQueue<int> queue{};
  TEST_ASSERT_TRUE(queue.empty());
  queue.push(1);
  TEST_ASSERT_FALSE(queue.empty());
}

void test_threaded() {
  malib::WaitableQueue<int> queue{};
  std::thread t([&queue]() {
    queue.push(1);
  });
  t.join();
  auto result = queue.pop();
  TEST_ASSERT_EQUAL(1, result);
}

void test_WaitableQueue() {
  RUN_TEST(test_push);
  RUN_TEST(test_pop);
  RUN_TEST(test_empty);
}