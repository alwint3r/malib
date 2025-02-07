#pragma once
#include <expected>
#include <functional>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "malib/FixedStringBuffer.hpp"
#include "malib/Token.hpp"
#include "malib/Tokenizer.hpp"
#include "malib/concepts.hpp"

namespace malib {
namespace shell {

using arguments = TokenViews;

template <byte_output_interface OutputBufferType = FixedStringBuffer<256>,
          std::size_t MaxTokens = 32>
struct tiny {
  using callback =
      std::function<Error(std::string_view, arguments, OutputBufferType&)>;
  using registry = std::map<std::string_view, callback>;

  void registerCommand(std::string_view name, callback cb) {
    registry_[name] = cb;
  }

  bool isCommandValid(std::string_view name) const {
    return registry_.find(name) != registry_.end();
  }

  Error execute(std::string_view input, output_interface auto& output) {
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

    auto command = tokenizer_[0]->view(input);
    if (!isCommandValid(command)) {
      output.write(invalid_command_message.data(),
                   invalid_command_message.size());
      return Error::InvalidCommand;
    }

    auto all_tokens = tokenizer_.tokens_span();
    arguments args{input, all_tokens.subspan(1)};

    auto command_cb = registry_[command];
    if (command_cb == nullptr) {
      output.write(no_command_message.data(), no_command_message.size());
      return Error::NullPointerMember;
    }

    output_buffer_.clear();
    auto command_result = command_cb(command, args, output_buffer_);
    if (command_result != Error::Ok) {
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