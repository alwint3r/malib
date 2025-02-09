#include <unity.h>

#include <algorithm>
#include <functional>
#include <malib/FixedSizeWaitableQueue.hpp>
#include <string>
#include <thread>
#include <vector>

void test_FixedSizeWaitableQueue_with_callback_functions() {
  malib::FixedSizeWaitableQueue<std::function<void(const std::string&)>, 2>
      queue;
  std::string result;

  // Test pushing and popping functions
  auto err = queue.push(
      [&result](const std::string& val) { result = "Hello " + val; });
  TEST_ASSERT_EQUAL(malib::Error::Ok, err);

  auto fn = queue.pop();
  TEST_ASSERT_TRUE(fn);
  fn("World");
  TEST_ASSERT_EQUAL_STRING("Hello World", result.c_str());

  // Test queue full behavior
  auto fn1 = [](const std::string&) {};
  auto fn2 = [](const std::string&) {};
  queue.push(fn1);
  queue.push(fn2);
  err = queue.push(fn1);  // Should fail, queue is full
  TEST_ASSERT_EQUAL(malib::Error::QueueFull, err);
}

void test_FixedSizeWaitableQueue_threaded() {
  constexpr size_t QUEUE_SIZE = 4;
  constexpr size_t NUM_ITEMS = 100;
  constexpr size_t NUM_PRODUCERS = 3;
  constexpr size_t NUM_CONSUMERS = 2;

  malib::FixedSizeWaitableQueue<int, QUEUE_SIZE> queue;
  std::vector<int> results;
  std::mutex results_mutex;

  // Producer threads
  std::vector<std::thread> producers;
  for (size_t i = 0; i < NUM_PRODUCERS; ++i) {
    producers.emplace_back([&queue, i]() {
      for (size_t j = 0; j < NUM_ITEMS; ++j) {
        int value = i * NUM_ITEMS + j;
        while (queue.push(value) == malib::Error::QueueFull) {
          std::this_thread::yield();
        }
      }
    });
  }

  // Consumer threads
  std::vector<std::thread> consumers;
  for (size_t i = 0; i < NUM_CONSUMERS; ++i) {
    consumers.emplace_back([&queue, &results, &results_mutex]() {
      for (size_t j = 0; j < (NUM_ITEMS * NUM_PRODUCERS) / NUM_CONSUMERS; ++j) {
        int value = queue.pop();
        std::lock_guard<std::mutex> lock(results_mutex);
        results.push_back(value);
      }
    });
  }

  // Join all threads
  for (auto& p : producers) p.join();
  for (auto& c : consumers) c.join();

  // Verify results
  TEST_ASSERT_EQUAL(NUM_ITEMS * NUM_PRODUCERS, results.size());
  std::sort(results.begin(), results.end());
  for (size_t i = 0; i < results.size(); ++i) {
    TEST_ASSERT_EQUAL(i, results[i]);
  }
}

void test_FixedSizeWaitableQueue_pop_blocks_on_empty() {
    malib::FixedSizeWaitableQueue<int, 2> queue;
    bool thread_finished = false;
    int received_value = 0;
    
    // Start thread that will block on pop
    std::thread consumer([&]() {
        received_value = queue.pop();
        thread_finished = true;
    });
    
    // Give thread time to reach blocking point
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify thread is blocked
    TEST_ASSERT_FALSE(thread_finished);
    
    // Unblock thread by pushing value
    queue.push(42);
    
    // Wait for thread to finish
    consumer.join();
    
    // Verify thread completed and got correct value
    TEST_ASSERT_TRUE(thread_finished);
    TEST_ASSERT_EQUAL(42, received_value);
}

void test_FixedSizeWaitableQueue() {
  RUN_TEST(test_FixedSizeWaitableQueue_with_callback_functions);
  RUN_TEST(test_FixedSizeWaitableQueue_threaded);
  RUN_TEST(test_FixedSizeWaitableQueue_pop_blocks_on_empty);
}