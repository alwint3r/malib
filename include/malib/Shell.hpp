#pragma once
#include <expected>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "malib/FixedStringBuffer.hpp"
#include "malib/Token.hpp"
#include "malib/Tokenizer.hpp"
#include "malib/concepts.hpp"

namespace malib {
namespace shell {

using arguments = std::vector<std::string_view>;

template <byte_output_interface OutputBufferType = FixedStringBuffer<256>>
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

    if (input.size() == 0) {
      return Error::EmptyInput;
    }

    auto tokenizer_result = tokenizer_.tokenize(input);
    if (tokenizer_result.has_value() == false) {
      return tokenizer_result.error();
    }

    auto command = tokenizer_[0]->view(input);
    if (isCommandValid(command) == false) {
      std::string_view error_message = "Invalid command\n";
      output.write(error_message.data(), error_message.size());
      return Error::InvalidCommand;
    }

    arguments args = tokenizer_.tokens_vector(input);
    args.erase(args.begin());

    auto command_cb = registry_[command];
    if (command_cb == nullptr) {
      std::string_view error_message = "Command has no executable code\n";
      output.write(error_message.data(), error_message.size());
      return Error::NullPointerMember;
    }

    auto command_result = command_cb(command, args, output_buffer_);
    if (command_result != Error::Ok) {
      return command_result;
    }

    auto output_result =
        output.write(output_buffer_.data(), output_buffer_.size());
    return output_result.has_value() ? Error::Ok : output_result.error();
  }

 private:
  registry registry_{};
  OutputBufferType output_buffer_{};
  Tokenizer<32> tokenizer_{};
  std::mutex mutex_{};
};
};  // namespace shell
};  // namespace malib