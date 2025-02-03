#include <unity.h>

#include <iostream>
#include <malib/Shell.hpp>

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
  shell.registerCommand(
      "echo",
      [](std::string command, malib::shell::arguments args) { return "echo"; });

  TEST_ASSERT_TRUE(shell.isCommandValid("echo"));
  TEST_ASSERT_FALSE(shell.isCommandValid("invalid"));
}

void test_execute() {
  malib::shell::tiny shell{};
  shell.registerCommand("echo",
                        [](std::string command, malib::shell::arguments args) {
                          std::string result = "";

                          for (const auto& arg : args) {
                            result += arg.view().value();
                            result += " ";
                          }
                          return result;
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
  TEST_ASSERT_EQUAL_STRING("Command has no executable code\n", output.output.c_str());
}

void test_Shell() {
  RUN_TEST(test_addCommand);
  RUN_TEST(test_execute);
  RUN_TEST(test_invalidCommand);
  RUN_TEST(test_emptyInput);
  RUN_TEST(test_nullCallback);
}