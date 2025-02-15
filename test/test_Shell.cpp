#include <unity.h>

#include <iostream>
#include <malib/Shell.hpp>
#include <thread>

struct stub_output {
  std::string output{};
  std::expected<std::size_t, malib::Error> write(const char* buf,
                                                 std::size_t size) {
    output = std::string(buf, size);
    return output.size();
  }
};

void test_addCommand() {
  malib::shell::tiny shell{};
  shell.registerCommand("test", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    return output.write("test", 4).error_or(malib::Error::Ok);
  });

  TEST_ASSERT_TRUE(shell.isCommandValid("test"));

  // Test command overwrite
  shell.registerCommand("test", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    return output.write("test2", 5).error_or(malib::Error::Ok);
  });

  stub_output output{};
  auto result = shell.execute("test", output);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_STRING("test2", output.output.c_str());
}

void test_execute() {
  malib::shell::tiny shell{};
  shell.registerCommand("echo", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    std::string result{};

    for (const auto& arg : args) {
      result += arg;
      result += " ";
    }

    return output.write(result.c_str(), result.size())
        .error_or(malib::Error::Ok);
  });

  stub_output output{};
  auto result = shell.execute("echo Hello World", output);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_STRING("Hello World ", output.output.c_str());
}

void test_invalidCommand() {
  malib::shell::tiny shell{};
  stub_output output{};
  auto result = shell.execute("invalidCommand", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidCommand, result);
  TEST_ASSERT_EQUAL_STRING("Invalid command\n", output.output.c_str());
}

void test_emptyInput() {
  malib::shell::tiny shell{};
  stub_output output{};
  auto result = shell.execute("", output);
  TEST_ASSERT_EQUAL(malib::Error::EmptyInput, result);
  TEST_ASSERT_EQUAL_STRING("", output.output.c_str());
}

void test_nullCallback() {
  malib::shell::tiny shell{};

  // Test registering null callback
  auto result = shell.registerCommand("nullcmd", nullptr);
  TEST_ASSERT_EQUAL(malib::Error::NullPointerMember, result);

  // Verify command was not registered
  TEST_ASSERT_FALSE(shell.isCommandValid("nullcmd"));

  stub_output output{};
  result = shell.execute("nullcmd", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidCommand, result);
  TEST_ASSERT_EQUAL_STRING("Invalid command\n", output.output.c_str());
}

void test_malformedInput() {
  malib::shell::tiny shell{};
  stub_output output{};
  auto result = shell.execute("echo \"unclosed quote", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidCommand, result);
}

void test_emptyArguments() {
  malib::shell::tiny shell{};
  shell.registerCommand("test", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    std::string result = std::to_string(args.size());
    return output.write(result.c_str(), result.size())
        .error_or(malib::Error::Ok);
  });

  stub_output output{};
  auto result = shell.execute("test", output);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_STRING("0", output.output.c_str());
}

void test_bufferOverflow() {
  malib::shell::tiny<malib::FixedStringBuffer<5>> shell{};
  shell.registerCommand(
      "overflow",
      [](std::string_view command, malib::shell::arguments args, auto& output) {
        TEST_ASSERT_EQUAL(5, output.capacity());
        TEST_ASSERT_EQUAL(0, output.size());
        auto res = output.write("too long", 8);
        TEST_ASSERT_FALSE(res.has_value());
        TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, res.error());
        TEST_ASSERT_EQUAL(0, output.size());

        return res.error_or(malib::Error::Ok);
      });

  stub_output output{};
  auto result = shell.execute("overflow", output);
  TEST_ASSERT_EQUAL(malib::Error::MaximumSizeExceeded, result);
}

void test_threadSafety() {
  malib::shell::tiny shell{};
  shell.registerCommand(
      "count",
      [](std::string_view command, malib::shell::arguments args, auto& output) {
        static int counter = 0;
        counter++;

        std::string output_str = std::to_string(counter);
        return output.write(output_str.c_str(), output_str.size())
            .error_or(malib::Error::Ok);
      });

  stub_output output1{}, output2{};
  std::thread t1([&]() { shell.execute("count", output1); });
  std::thread t2([&]() { shell.execute("count", output2); });

  t1.join();
  t2.join();

  // Values should be different due to mutex protection
  TEST_ASSERT_NOT_EQUAL(output1.output, output2.output);
}

void test_emptyCommandName() {
  malib::shell::tiny shell{};

  // Test registering empty command name
  auto result = shell.registerCommand(
      "", [](std::string_view command, malib::shell::arguments args,
             auto& output) { return malib::Error::Ok; });
  TEST_ASSERT_EQUAL(malib::Error::EmptyInput, result);

  // Verify command was not registered
  TEST_ASSERT_FALSE(shell.isCommandValid(""));
}

void test_commandCaseInsensitive() {
  malib::shell::tiny shell{};
  shell.registerCommand("ECHO", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    return output.write("ok", 2).error_or(malib::Error::Ok);
  });

  stub_output output{};
  auto result = shell.execute("echo", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidCommand,
                    result);  // Should be case sensitive
}

void test_concurrentCommandExecution() {
  malib::shell::tiny shell{};
  int counter = 0;
  shell.registerCommand(
      "increment", [&counter](std::string_view command,
                              malib::shell::arguments args, auto& output) {
        counter++;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));  // Simulate work
        return output.write(std::to_string(counter).c_str(), 1)
            .error_or(malib::Error::Ok);
      });

  std::vector<std::thread> threads;
  std::vector<stub_output> outputs(5);

  for (int i = 0; i < 5; i++) {
    threads.emplace_back(
        [&shell, &outputs, i]() { shell.execute("increment", outputs[i]); });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  TEST_ASSERT_EQUAL(5, counter);  // Should be exactly 5 due to mutex protection
}

void test_commandFailureWithOutput() {
  malib::shell::tiny shell{};
  shell.registerCommand("fail", [](std::string_view command,
                                   malib::shell::arguments args, auto& output) {
    output.write("Error occurred!", 16);
    return malib::Error::InvalidArgument;
  });

  stub_output output{};
  auto result = shell.execute("fail", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidArgument, result);
  TEST_ASSERT_EQUAL_STRING("Error occurred!", output.output.c_str());
}

void test_outputBufferOverflow() {
  malib::shell::tiny<malib::FixedLengthLinearBuffer<char, 8>> shell{};
  shell.registerCommand(
      "overflow",
      [](std::string_view command, malib::shell::arguments args, auto& output) {
        auto res = output.write("AAAAAAA", 8);
        TEST_ASSERT_TRUE(res.has_value());
        TEST_ASSERT_EQUAL(8, res.value());

        res = output.write("1234", 4);
        TEST_ASSERT_FALSE(res.has_value());
        TEST_ASSERT_EQUAL(malib::Error::BufferFull, res.error());

        return res.error_or(malib::Error::Ok);
      });

  stub_output output{};
  auto result = shell.execute("overflow", output);
  TEST_ASSERT_EQUAL(malib::Error::BufferFull, result);
  TEST_ASSERT_EQUAL(8, output.output.size());
  TEST_ASSERT_EQUAL_STRING("AAAAAAA", output.output.c_str());
}

void test_Shell() {
  RUN_TEST(test_addCommand);
  RUN_TEST(test_execute);
  RUN_TEST(test_invalidCommand);
  RUN_TEST(test_emptyInput);
  RUN_TEST(test_nullCallback);
  RUN_TEST(test_malformedInput);
  RUN_TEST(test_emptyArguments);
  RUN_TEST(test_bufferOverflow);
  RUN_TEST(test_threadSafety);
  RUN_TEST(test_emptyCommandName);
  RUN_TEST(test_commandCaseInsensitive);
  RUN_TEST(test_concurrentCommandExecution);
  RUN_TEST(test_outputBufferOverflow);
  RUN_TEST(test_commandFailureWithOutput);
}