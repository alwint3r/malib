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
  shell.registerCommand("test", [](const std::string& command,
                                   malib::shell::arguments args, auto& output) {
    output.reset();
    return output.write("test", 4).error_or(malib::Error::Ok);
  });

  TEST_ASSERT_TRUE(shell.isCommandValid("test"));

  // Test command overwrite
  shell.registerCommand("test", [](const std::string& command,
                                   malib::shell::arguments args, auto& output) {
    output.reset();
    return output.write("test2", 5).error_or(malib::Error::Ok);
  });

  stub_output output{};
  auto result = shell.execute("test", output);
  TEST_ASSERT_EQUAL(malib::Error::Ok, result);
  TEST_ASSERT_EQUAL_STRING("test2", output.output.c_str());
}

void test_execute() {
  malib::shell::tiny shell{};
  shell.registerCommand("echo", [](const std::string& command,
                                   malib::shell::arguments args, auto& output) {
    output.reset();
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
  shell.registerCommand("nullcmd", nullptr);

  stub_output output{};
  auto result = shell.execute("nullcmd", output);
  TEST_ASSERT_EQUAL(malib::Error::NullPointerMember, result);
  TEST_ASSERT_EQUAL_STRING("Command has no executable code\n",
                           output.output.c_str());
}

void test_malformedInput() {
  malib::shell::tiny shell{};
  stub_output output{};
  auto result = shell.execute("echo \"unclosed quote", output);
  TEST_ASSERT_EQUAL(malib::Error::InvalidCommand, result);
}

void test_emptyArguments() {
  malib::shell::tiny shell{};
  shell.registerCommand("test", [](const std::string& command,
                                   malib::shell::arguments args, auto& output) {
    output.reset();
    return output.write(std::to_string(args.size()).c_str(), 1)
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
      "overflow", [](const std::string& command, malib::shell::arguments args,
                     auto& output) {
        output.reset();
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
      "count", [](const std::string& command, malib::shell::arguments args,
                  auto& output) {
        output.reset();
        static int counter = 0;
        counter++;
        return output.write(std::to_string(counter).c_str(), 1)
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
}