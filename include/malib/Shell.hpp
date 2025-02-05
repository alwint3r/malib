#pragma once
#include <expected>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "malib/Token.hpp"
#include "malib/Tokenizer.hpp"
#include "malib/concepts.hpp"

namespace malib {
namespace shell {

using arguments = std::vector<Token>;
using callback = std::function<std::string(std::string, arguments)>;
using registry = std::map<std::string, callback>;

struct tiny {
  void registerCommand(const std::string& name, callback cb) {
    registry_[name] = cb;
  }

  bool isCommandValid(const std::string& name) const {
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

    auto command = std::string(tokenizer_[0].value().view().value());
    if (isCommandValid(command) == false) {
      std::string_view error_message = "Invalid command\n";
      output.write(error_message.data(), error_message.size());
      return Error::InvalidCommand;
    }

    arguments args = tokenizer_.tokens_vector();
    args.erase(args.begin());

    auto command_cb = registry_[command];
    if (command_cb == nullptr) {
      std::string_view error_message = "Command has no executable code\n";
      output.write(error_message.data(), error_message.size());
      return Error::NullPointerMember;
    }
    auto result = command_cb(command, args);
    auto output_result = output.write(result.c_str(), result.size());
    return output_result.has_value() ? Error::Ok : output_result.error();
  }

 private:
  registry registry_{};
  Tokenizer<32> tokenizer_{};
  std::mutex mutex_{};
};
};  // namespace shell
};  // namespace malib