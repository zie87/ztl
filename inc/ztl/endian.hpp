// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_ENDIAN_HPP
#define ZTL_ENDIAN_HPP

#include "ztl/type_traits.hpp"

#include <type_traits>
#include <cstdint>

namespace ztl
{

// NOTE: inspired by std::endian (backport of C++20)
enum class endian
{
    little = 0,
    big    = 1,
    // WARN: this works because we only work on little endian machines so far
    native = little
};

constexpr inline auto reverse_bytes(std::uint8_t value) noexcept -> std::uint8_t { return value; }

constexpr inline auto reverse_bytes(std::uint16_t value) noexcept -> std::uint16_t
{
    return (value >> 8U) | (value << 8U);
}

constexpr inline auto reverse_bytes(std::uint32_t value) noexcept -> std::uint32_t
{
    value = ((value & 0xFF00FF00UL) >> 8U) | ((value & 0x00FF00FFUL) << 8U);
    value = (value >> 16U) | (value << 16U);

    return value;
}

constexpr inline auto reverse_bytes(std::uint64_t value) noexcept -> std::uint64_t
{
    return ((value & 0x00000000000000ffull) << 56u) | ((value & 0x000000000000ff00ull) << 40u)
         | ((value & 0x0000000000ff0000ull) << 24u) | ((value & 0x00000000ff000000ull) << 8u)
         | ((value & 0x000000ff00000000ull) >> 8u) | ((value & 0x0000ff0000000000ull) >> 24u)
         | ((value & 0x00ff000000000000ull) >> 40u) | ((value & 0xff00000000000000ull) >> 56u);
}

// NOTE: backport from C++23
template<typename T>
constexpr inline typename std::enable_if<std::is_integral<T>::value, T>::type
byteswap(T value) noexcept
{
    return reverse_bytes(value);
}

template<endian FROM_V, endian TO_V>
struct endian_swapper
{
    template<typename T>
    static inline constexpr T endian_swap(T value) noexcept
    {
        return byteswap(value);
    }
};

template<endian ENDIAN_V>
struct endian_swapper<ENDIAN_V, ENDIAN_V>
{
    template<typename T>
    static inline constexpr T endian_swap(T value) noexcept
    {
        return value;
    }
};

template<endian FROM_V, endian TO_V, typename T>
inline constexpr ztl::remove_cvref_t<T> endian_swap(T value) noexcept
{
    using swapper = endian_swapper<FROM_V, TO_V>;
    return swapper::endian_swap(value);
}

} // namespace ztl

#endif // ZTL_ENDIAN_HPP
