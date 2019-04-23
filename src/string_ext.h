#pragma once

#include <string_view>

// Shims for std::string/std::string_view's starts_with()/ends_with() methods for C++17.

template<typename CharT>
constexpr bool
string_starts_with(std::basic_string_view<CharT> haystack, std::basic_string_view<CharT> needle) noexcept {
    return haystack.size() >= needle.size() && haystack.compare(0, needle.size(), needle) == 0;
}

template<typename CharT>
constexpr bool string_starts_with(std::basic_string_view<CharT> haystack, CharT needle) noexcept {
    return string_starts_with(haystack, std::basic_string_view<CharT>(&needle, 1));
}

template<typename CharT>
constexpr bool string_starts_with(std::basic_string_view<CharT> haystack, CharT const* needle) {
    return string_starts_with(haystack, std::basic_string_view<CharT>(needle));
}

template<typename CharT>
constexpr bool string_ends_with(std::basic_string_view<CharT> haystack, std::basic_string_view<CharT> needle) noexcept {
    return haystack.size() >= needle.size()
        && haystack.compare(haystack.size() - needle.size(), std::basic_string_view<CharT>::npos, needle) == 0;
}

template<typename CharT>
constexpr bool string_ends_with(std::basic_string_view<CharT> haystack, CharT needle) noexcept {
    return string_ends_with(haystack, std::basic_string_view<CharT>(&needle, 1));
}

template<typename CharT>
constexpr bool string_ends_with(std::basic_string_view<CharT> haystack, CharT const* needle) {
    return string_ends_with(haystack, std::basic_string_view<CharT>(needle));
}

template<typename CharT, typename Traits>
constexpr bool string_starts_with(
    const std::basic_string<CharT, Traits>& haystack,
    const std::basic_string<CharT, Traits>& needle
) noexcept {
    return string_starts_with(std::basic_string_view<CharT>(haystack), std::basic_string_view<CharT>(needle));
}

template<typename CharT, typename Traits>
constexpr bool string_starts_with(const std::basic_string<CharT, Traits>& haystack, CharT needle) noexcept {
    return string_starts_with(std::basic_string_view<CharT>(haystack), needle);
}

template<typename CharT, typename Traits>
constexpr bool string_starts_with(const std::basic_string<CharT, Traits>& haystack, CharT const* needle) {
    return string_starts_with(std::basic_string_view<CharT>(haystack), needle);
}

template<typename CharT, typename Traits>
constexpr bool string_ends_with(
    const std::basic_string<CharT, Traits>& haystack,
    const std::basic_string<CharT, Traits>& needle
) noexcept {
    return string_ends_with(std::basic_string_view<CharT>(haystack), std::basic_string_view<CharT>(needle));
}

template<typename CharT, typename Traits>
constexpr bool string_ends_with(const std::basic_string<CharT, Traits>& haystack, CharT needle) noexcept {
    return string_ends_with(std::basic_string_view<CharT>(haystack), needle);
}

template<typename CharT, typename Traits>
constexpr bool string_ends_with(const std::basic_string<CharT, Traits>& haystack, CharT const* needle) {
    return string_ends_with(std::basic_string_view<CharT>(haystack), needle);
}
