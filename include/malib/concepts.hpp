#pragma once

#include <concepts>
#include <expected>
#include <string>
#include <type_traits>

namespace malib {

// strings
template <typename T>
concept cstyle_string =
    std::is_pointer_v<T> &&
    std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>;

template <typename T>
concept string_like =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

static_assert(string_like<std::string>);
static_assert(string_like<std::string_view>);
static_assert(cstyle_string<const char *>);
static_assert(cstyle_string<char *>);
static_assert(cstyle_string<void *> == false);

// buffers
template <typename B>
concept buffer_like = requires(B t) {
  typename B::ValueType;
  { t.push(std::declval<typename B::ValueType>()) } -> std::same_as<Error>;
  { t.pop() } -> std::same_as<std::expected<typename B::ValueType, Error>>;
  { t.peek() } -> std::same_as<std::expected<typename B::ValueType, Error>>;
  { t.size() } -> std::same_as<size_t>;
  { t.empty() } -> std::same_as<bool>;
  { t.clear() } -> std::same_as<void>;
};

// interfaces
template <typename T>
concept output_interface = requires(T t) {
  {
    t.write(std::declval<const char *>(), std::declval<std::size_t>())
  } -> std::same_as<std::expected<std::size_t, Error>>;
};

}  // namespace malib