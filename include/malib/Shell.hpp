#pragma once
#include <expected>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "malib/Token.hpp"
#include "malib/Tokenizer.hpp"

namespace malib {
namespace shell {

template <typename T>
concept output_interface = requires(T t) {
  { t.write("Hello", 5) } -> std::same_as<std::expected<std::size_t, Error>>;
};

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

    auto tokens_count = tokenizer_result.value();

    arguments args{};
    args.reserve(tokens_count);

    auto command = std::string(tokenizer_[0].value().view().value());
    if (isCommandValid(command) == false) {
      return Error::InvalidCommand;
    }

    for (std::size_t i = 1; i < tokens_count; i++) {
      auto token = tokenizer_[i];
      if (token.has_value() == false) {
        return token.error();
      }
      args.push_back(token.value());
    }

    auto command_cb = registry_[command];
    if (command_cb == nullptr) {
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