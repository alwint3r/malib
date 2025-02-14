#include <unity.h>

#include <atomic>
#include <thread>

#include "malib/RingBuffer.hpp"

void test_push_pop() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(1));
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(2));
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(3));
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, buffer.push(4));

  auto result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());

  result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(2, result.value());

  result = buffer.pop();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());

  result = buffer.pop();
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferEmpty, result.error());
}

void test_clear() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  buffer.push(1);
  buffer.push(2);
  buffer.clear();
  TEST_ASSERT_TRUE(buffer.empty());
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(3));
}

void test_size() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  TEST_ASSERT_EQUAL(0, buffer.size());
  buffer.push(1);
  TEST_ASSERT_EQUAL(1, buffer.size());
  buffer.push(2);
  TEST_ASSERT_EQUAL(2, buffer.size());
  buffer.pop();
  TEST_ASSERT_EQUAL(1, buffer.size());
}

void test_wraparound() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};

  // Fill buffer
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);

  // Pop two items
  buffer.pop();
  buffer.pop();

  // Push new items to trigger wraparound
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(4));
  TEST_ASSERT_EQUAL(malib::Error::Ok, buffer.push(5));

  auto result = buffer.pop();
  TEST_ASSERT_EQUAL(3, result.value());
  result = buffer.pop();
  TEST_ASSERT_EQUAL(4, result.value());
  result = buffer.pop();
  TEST_ASSERT_EQUAL(5, result.value());
}

void test_different_types() {
  using SmallRingBuffer = malib::RingBuffer<std::string, 2>;
  SmallRingBuffer str_buffer{};
  TEST_ASSERT_EQUAL(malib::Error::Ok, str_buffer.push("test1"));
  TEST_ASSERT_EQUAL(malib::Error::Ok, str_buffer.push("test2"));

  auto result = str_buffer.pop();
  TEST_ASSERT_EQUAL_STRING("test1", result.value().c_str());
}

void test_concurrent_access() {
  using SmallRingBuffer = malib::RingBuffer<int, 100>;
  SmallRingBuffer buffer{};

  std::atomic<int> pushed_count{0};
  std::atomic<int> popped_count{0};
  std::atomic<bool> producer_done{false};

  std::thread producer([&]() {
    for (int i = 0; i < 50; i++) {
      while (buffer.push(i) == malib::Error::BufferFull) {
        std::this_thread::yield();
      }
      pushed_count++;
    }
    producer_done = true;
  });

  std::thread consumer([&]() {
    while (!producer_done || !buffer.empty()) {
      auto result = buffer.pop();
      if (result.has_value()) {
        popped_count++;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  // Meaningful assertions
  TEST_ASSERT_EQUAL(50, pushed_count);
  TEST_ASSERT_EQUAL(50, popped_count);
  TEST_ASSERT_TRUE(buffer.empty());
}

void test_peek() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);

  auto result = buffer.peek();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(1, result.value());

  buffer.pop();
  result = buffer.peek();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(2, result.value());

  buffer.pop();
  result = buffer.peek();
  TEST_ASSERT_TRUE(result.has_value());
  TEST_ASSERT_EQUAL(3, result.value());

  buffer.pop();
  result = buffer.peek();
  TEST_ASSERT_FALSE(result.has_value());
  TEST_ASSERT_EQUAL(malib::Error::BufferEmpty, result.error());
}

void test_consume_all() {
  using SmallRingBuffer = malib::RingBuffer<int, 3>;
  SmallRingBuffer buffer{};
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);

  auto [elements, count] = buffer.consume_all();
  TEST_ASSERT_EQUAL(3, count);
  TEST_ASSERT_EQUAL(1, elements[0]);
  TEST_ASSERT_EQUAL(2, elements[1]);
  TEST_ASSERT_EQUAL(3, elements[2]);
  TEST_ASSERT_TRUE(buffer.empty());

  SmallRingBuffer empty_buffer{};
  auto [empty_elements, empty_count] = empty_buffer.consume_all();
  TEST_ASSERT_EQUAL(0, empty_count);
  TEST_ASSERT_TRUE(empty_buffer.empty());
}

void test_overwrite_policy() {
  // Test discard policy (default)
  using DefaultBuffer = malib::RingBuffer<int, 3>;
  DefaultBuffer default_buffer{};
  TEST_ASSERT_EQUAL(malib::Error::Ok, default_buffer.push(1));
  TEST_ASSERT_EQUAL(malib::Error::Ok, default_buffer.push(2));
  TEST_ASSERT_EQUAL(malib::Error::Ok, default_buffer.push(3));
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, default_buffer.push(4));

  // Test overwrite policy
  using OverwriteBuffer =
      malib::RingBuffer<int, 3, malib::OverwritePolicy::Overwrite>;
  OverwriteBuffer overwrite_buffer{};

  // Fill buffer
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(1));
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(2));
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(3));

  // Should overwrite first element
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(4));

  auto result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(2, result.value());

  result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(3, result.value());

  result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(4, result.value());

  TEST_ASSERT_TRUE(overwrite_buffer.empty());

  // Test multiple overwrites
  overwrite_buffer.clear();
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(1));
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(2));
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(3));

  // Overwrite two values
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(4));
  TEST_ASSERT_EQUAL(malib::Error::Ok, overwrite_buffer.push(5));

  result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(3, result.value());

  result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(4, result.value());

  result = overwrite_buffer.pop();
  TEST_ASSERT_EQUAL(5, result.value());

  TEST_ASSERT_TRUE(overwrite_buffer.empty());
}

void test_overwrite_policy_sequence() {
  malib::RingBuffer<int, 4, malib::OverwritePolicy::Overwrite> buffer;

  // Fill buffer
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);
  buffer.push(4);

  // Overwrite twice
  buffer.push(5);
  buffer.push(6);

  // Expected sequence: 3,4,5,6
  // Without tail_ = head_, you might get: 5,2,3,4 or other incorrect sequences
  auto [elements, count] = buffer.consume_all();
  TEST_ASSERT_EQUAL(count, 4);
  TEST_ASSERT_EQUAL(elements[0], 3);
  TEST_ASSERT_EQUAL(elements[1], 4);
  TEST_ASSERT_EQUAL(elements[2], 5);
  TEST_ASSERT_EQUAL(elements[3], 6);
}

void test_overwrite_policy_sequence_pop() {
  malib::RingBuffer<int, 3, malib::OverwritePolicy::Overwrite> buffer;

  buffer.push(1);
  buffer.push(2);
  buffer.push(3);  // [1,2,3] h=0, t=0
  buffer.push(4);  // [4,2,3] h=1, t=1 or t=0
  buffer.push(5);  // [4,5,3] h=2, t=2 or t=0

  auto val1 = buffer.pop();
  auto val2 = buffer.pop();
  auto val3 = buffer.pop();

  TEST_ASSERT_EQUAL(val1.value(), 3);
  TEST_ASSERT_EQUAL(val2.value(), 4);
  TEST_ASSERT_EQUAL(val3.value(), 5);
}

void test_RingBuffer() {
  RUN_TEST(test_push_pop);
  RUN_TEST(test_clear);
  RUN_TEST(test_size);
  RUN_TEST(test_wraparound);
  RUN_TEST(test_different_types);
  RUN_TEST(test_concurrent_access);
  RUN_TEST(test_peek);
  RUN_TEST(test_consume_all);
  RUN_TEST(test_overwrite_policy);           // Add new test to the suite
  RUN_TEST(test_overwrite_policy_sequence);  // Add new test to the suite
}