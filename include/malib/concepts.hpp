#pragma once

#include <concepts>
#include <expected>
#include <string>
#include <type_traits>
#include <system_error>

namespace malib {

// error type
template <typename T>
concept error_type = requires(T t) {
  requires std::convertible_to<T, int> or std::is_enum_v<T> or
               std::convertible_to<T, std::error_code>;
};
static_assert(error_type<Error>);

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
  typename B::value_type;
  { t.push(std::declval<typename B::value_type>()) } -> std::same_as<Error>;
  { t.pop() } -> std::same_as<std::expected<typename B::value_type, Error>>;
  { t.peek() } -> std::same_as<std::expected<typename B::value_type, Error>>;
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

template <typename T, typename ErrType = Error>
concept input_interface = requires(T t, char *buffer, std::size_t size) {
  { t.read(buffer, size) } -> std::same_as<std::expected<std::size_t, ErrType>>;
} || requires(T t, char *buffer, std::size_t size) {
  { t.read(buffer, size) } -> std::same_as<std::size_t>;
};

}  // namespace malib