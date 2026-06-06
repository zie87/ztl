// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_BYTE_HPP
#define ZTL_BYTE_HPP

#include "ztl/assert.hpp"
#include "ztl/narrow.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#if defined(__cpp_lib_byte) && (__cpp_lib_byte >= 201603L)

namespace ztl {
using byte = std::byte;

static_assert(sizeof(ztl::byte) == 1 && alignof(ztl::byte) == 1, "ztl::byte must be a true byte type");
} // namespace ztl

#else

// NOTE: ztl::byte is backport of C++17 std::byte, as a scoped
// enum over uint8_t (≡ unsigned char). Strict aliasing exemption
// applies in practice on all supported targets. This matches
// the intent of std::byte (C++17 [basic.lval]).

namespace ztl {
// clang-format off
enum struct byte : std::uint8_t {};
static_assert(sizeof(ztl::byte) == 1 && alignof(ztl::byte) == 1, "ztl::byte must be a true byte type");
// clang-format on
} // namespace ztl

#endif

namespace ztl {

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr T to_integer(byte b) noexcept {
    return static_cast<T>(b);
}

static inline constexpr auto to_byte(std::uint8_t value) noexcept -> byte {
    return static_cast<ztl::byte>(value);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte operator<<(byte b, T shift) noexcept {
    return byte(to_underlying(b) << shift);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte& operator<<=(byte& b, T shift) noexcept {
    return b = (b << shift);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte operator>>(byte b, T shift) noexcept {
    return byte(to_underlying(b) >> shift);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte& operator>>=(byte& b, T shift) noexcept {
    return b = (b >> shift);
}

constexpr byte operator|(byte l, byte r) noexcept {
    return byte(to_underlying(l) | to_underlying(r));
}

constexpr byte& operator|=(byte& l, byte r) noexcept {
    return l = (l | r);
}

constexpr byte operator&(byte l, byte r) noexcept {
    return byte(to_underlying(l) & to_underlying(r));
}

constexpr byte& operator&=(byte& l, byte r) noexcept {
    return l = (l & r);
}

constexpr byte operator^(byte l, byte r) noexcept {
    return byte(to_underlying(l) ^ to_underlying(r));
}

constexpr byte& operator^=(byte& l, byte r) noexcept {
    return l = (l ^ r);
}

constexpr byte operator~(byte b) noexcept {
    return byte(static_cast<std::uint8_t>(~to_underlying(b)));
}

namespace details {
template <typename T>
inline constexpr bool is_valid_byte_bit_position(T pos) noexcept {
    return std::is_signed<T>::value ? (pos >= 0 && pos < 8) : (pos < 8);
}
} // namespace details

// NOTE: backport of std::bit_cast (C++20)
template <typename TARGET_T, typename SOURCE_T>
inline typename std::enable_if<sizeof(TARGET_T) == sizeof(SOURCE_T) && std::is_trivially_copyable<SOURCE_T>::value &&
                                   std::is_trivially_copyable<TARGET_T>::value,
                               TARGET_T>::type
bit_cast(const SOURCE_T& src) noexcept {
    static_assert(std::is_default_constructible<TARGET_T>::value,
                  "ztl::bit_cast<T>() requires default-constructible target in this C++14 implementation");

    TARGET_T dst;
    std::memcpy(&dst, &src, sizeof(TARGET_T));
    return dst;
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr bool get_bit(byte b, T pos) noexcept {
    ZTL_EXPECTS(details::is_valid_byte_bit_position(pos));
    return ((b >> pos) & static_cast<byte>(1u)) == static_cast<byte>(1u);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void clear_bit(byte& b, T pos) noexcept {
    ZTL_EXPECTS(details::is_valid_byte_bit_position(pos));
    b &= ~(static_cast<byte>(1u) << pos);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void set_bit(byte& b, T pos) noexcept {
    ZTL_EXPECTS(details::is_valid_byte_bit_position(pos));
    b |= (static_cast<byte>(1u) << pos);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void set_bit(byte& b, T pos, bool val) noexcept {
    ZTL_EXPECTS(details::is_valid_byte_bit_position(pos));
    if (val) {
        set_bit(b, pos);
    } else {
        clear_bit(b, pos);
    }
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void toggle_bit(byte& b, T pos) noexcept {
    b ^= (static_cast<byte>(1u) << pos);
}

} // namespace ztl

#endif // ZTL_BYTE_HPP
