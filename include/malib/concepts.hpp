#pragma once

#include <concepts>
#include <expected>
#include <string>
#include <system_error>
#include <type_traits>

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
concept c_str =
    std::is_pointer_v<T> &&
    std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>;

template <typename T>
concept std_string =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

static_assert(std_string<std::string>);
static_assert(std_string<std::string_view>);
static_assert(c_str<const char *>);
static_assert(c_str<char *>);
static_assert(c_str<void *> == false);

template <typename Type, typename RetType>
concept std_expected_any_error = requires {
  typename std::remove_cvref_t<Type>::value_type;
  typename std::remove_cvref_t<Type>::error_type;
} && std::same_as<typename std::remove_cvref_t<Type>::value_type, RetType>;

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

template <typename T>
concept poppable_container = requires(T t) {
  { t.pop() } -> std_expected_any_error<typename T::value_type>;
};

template <typename T>
concept container_like = requires(T t) {
  typename T::value_type;
  { t.size() } -> std::same_as<std::size_t>;
  { t.empty() } -> std::same_as<bool>;
};

// interfaces
template <typename T, typename ErrType = Error>
concept byte_input_interface = requires(T t, char *buffer, std::size_t size) {
  { t.read(buffer, size) } -> std::same_as<std::expected<std::size_t, ErrType>>;
} || requires(T t, char *buffer, std::size_t size) {
  { t.read(buffer, size) } -> std::same_as<std::size_t>;
};

/// @brief A concept that defines a byte-oriented output interface
/// @details Types satisfying this concept must provide a write method that takes a pointer
/// to raw bytes and a size. The write method must return either:
/// - std::expected<std::size_t, E> where E is any error type, or
/// - A type convertible to std::size_t (for APIs that don't use expected)
/// The return value indicates the number of bytes actually written.
template <typename T>
concept byte_output_interface = requires(T t) {
  {
    t.write(std::declval<const char *>(), std::declval<std::size_t>())
  } -> std_expected_any_error<std::size_t>;
} || requires(T t) {
  {
    t.write(std::declval<const char *>(), std::declval<std::size_t>())
  } -> std::convertible_to<std::size_t>;
};

/// @brief A concept that defines a string-oriented output interface
/// @details Types satisfying this concept must provide a write method that takes a
/// std::string_view. The write method must return either:
/// - std::expected<std::size_t, E> where E is any error type, or
/// - A type convertible to std::size_t (for APIs that don't use expected)
/// The return value indicates the number of characters actually written.
template <typename T>
concept string_output_interface = requires(T t) {
  {
    t.write(std::declval<std::string_view>())
  } -> std_expected_any_error<std::size_t>;
} || requires(T t) {
  {
    t.write(std::declval<std::string_view>())
  } -> std::convertible_to<std::size_t>;
};

/// @brief A concept that combines both byte and string output interfaces
/// @details Types satisfying this concept must implement both byte_output_interface
/// and string_output_interface. This means they must provide:
/// - write(const char*, std::size_t) for raw byte output
/// - write(std::string_view) for string output
/// This concept is useful for types that need to handle both binary and text data.
template <typename T>
concept output_interface = byte_output_interface<T> && string_output_interface<T>;

template <typename T>
concept raw_accessible = requires(T t) {
  typename T::value_type;
  { t.data() } -> std::same_as<typename T::value_type *>;
  { t.size() } -> std::same_as<std::size_t>;
};

}  // namespace malib
