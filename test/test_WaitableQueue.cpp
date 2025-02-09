#include <unity.h>

#include <chrono>
#include <malib/WaitableQueue.hpp>
#include <thread>

void test_push() {
  malib::WaitableQueue<int> queue{};
  queue.push(1);
  TEST_ASSERT_FALSE(queue.empty());
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
  bool value_received = false;

  // Start consumer thread that will wait for data
  std::thread consumer([&queue, &value_received]() {
    auto result = queue.pop();  // This should block until data is available
    TEST_ASSERT_EQUAL(42, result);
    value_received = true;
  });

  // Small delay to ensure consumer is waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Producer thread
  std::thread producer([&queue]() { queue.push(42); });

  producer.join();
  consumer.join();

  TEST_ASSERT_TRUE(value_received);
}

void test_WaitableQueue() {
  RUN_TEST(test_push);
  RUN_TEST(test_pop);
  RUN_TEST(test_empty);
  RUN_TEST(test_threaded);
}