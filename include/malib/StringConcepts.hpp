#pragma once

#include <cstring>
#include <string>
#include <string_view>

namespace malib {

template <typename T>
concept CStyleString =
    std::is_pointer_v<T> &&
    std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>;

template <typename T>
concept StringLike =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

static_assert(StringLike<std::string>);
static_assert(StringLike<std::string_view>);
static_assert(CStyleString<const char *>);
static_assert(CStyleString<char *>);
static_assert(CStyleString<void *> == false);

}  // namespace malib
