#pragma once
#include <expected>
#include <functional>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "malib/FixedLengthLinearBuffer.hpp"
#include "malib/FixedStringBuffer.hpp"
#include "malib/Token.hpp"
#include "malib/Tokenizer.hpp"
#include "malib/concepts.hpp"

namespace malib {
namespace shell {

using arguments = TokenViews;

template <byte_output_interface OutputBufferType =
              FixedLengthLinearBuffer<char, 256>,
          std::size_t MaxTokens = 32>
struct tiny {
  using callback =
      std::function<Error(std::string_view, arguments, OutputBufferType&)>;
  using registry = std::map<std::string_view, callback>;

  /**
   * @brief Registers a command with the specified name and callback function.
   * 
   * This method allows adding new commands to the shell's command registry.
   * The command name must not be empty and the callback function must be valid.
   * Thread-safe registration is ensured through mutex protection.
   *
   * @param name The name of the command to register
   * @param cb The callback function to execute when the command is invoked
   * @return Error::Ok if registration succeeds
   * @return Error::EmptyInput if the command name is empty
   * @return Error::NullPointerMember if the callback is nullptr
   *
   */
  Error registerCommand(std::string_view name, callback cb) {
    if (name.empty()) {
      return Error::EmptyInput;
    }

    if (cb == nullptr) {
      return Error::NullPointerMember;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    registry_[name] = std::move(cb);
    return Error::Ok;
  }

  /**
   * @brief Checks if a command exists in the registry
   * @param name The name of the command to check
   * @return true if the command exists, false otherwise
   *
   */
  bool isCommandValid(std::string_view name) const {
    return registry_.find(name) != registry_.end();
  }

  /**
   * @brief Executes a shell command with the given input and writes output to the provided interface
   *
   * This function processes the input string, tokenizes it, validates the command,
   * and executes the corresponding callback if found in the registry.
   *
   * @tparam byte_output_interface Auto-deduced output interface type that supports write operations
   * 
   * @param input The command string to be executed
   * @param output The output interface where the command's result will be written
   *
   * @return Error code indicating the execution status:
   *         - Error::Ok on successful execution
   *         - Error::EmptyInput if input string is empty
   *         - Error::InvalidCommand if command is not valid
   *         - Error::NullPointerMember if command has no registered callback
   *         - Tokenizer-specific errors if tokenization fails
   *         - Command-specific errors if command execution fails
   *         - Output interface specific errors if writing fails
   *
   *
   * @note The function uses an internal buffer to store command output before writing
   *       to the provided output interface
   *
   */
  Error execute(std::string_view input, byte_output_interface auto& output) {
    std::lock_guard<std::mutex> lock(mutex_);
    static constexpr std::string_view invalid_command_message =
        "Invalid command\n";
    static constexpr std::string_view no_command_message =
        "Command has no executable code\n";

    if (input.empty()) {
      return Error::EmptyInput;
    }

    auto tokenizer_result = tokenizer_.tokenize(input);
    if (!tokenizer_result.has_value()) {
      return tokenizer_result.error();
    }

    auto command = tokenizer_.tokens_span()[0].view(input);
    if (!isCommandValid(command)) {
      output.write(invalid_command_message.data(),
                   invalid_command_message.size());
      return Error::InvalidCommand;
    }

    auto args = TokenViews::create(input, tokenizer_.tokens_span().subspan(1));
    if (!args.has_value()) {
      return args.error();
    }

    auto command_cb = registry_[command];
    if (command_cb == nullptr) {
      output.write(no_command_message.data(), no_command_message.size());
      return Error::NullPointerMember;
    }

    output_buffer_.clear();
    auto command_result = command_cb(command, *args, output_buffer_);
    if (command_result != Error::Ok) {
      output.write(output_buffer_.data(), output_buffer_.size());
      return command_result;
    }

    auto output_result =
        output.write(output_buffer_.data(), output_buffer_.size());
    return output_result.error_or(Error::Ok);
  }

 private:
  registry registry_{};
  OutputBufferType output_buffer_{};
  Tokenizer<MaxTokens> tokenizer_{};
  std::mutex mutex_{};
};
};  // namespace shell
};  // namespace malib