// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_SERIALIZE_HPP
#define ZTL_SERIALIZE_HPP

#include "ztl/endian.hpp"
#include "ztl/span.hpp"
#include "ztl/assert.hpp"
#include "ztl/utility.hpp"

#include <cstring>
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <array>

namespace ztl {

namespace details {
template <typename T, std::size_t EXTENT_V, std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
inline constexpr auto write_to_buffer(T value, ztl::span<ztl::byte, EXTENT_V> buffer) noexcept -> std::size_t {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
    static_assert(EXTENT_V == ztl::dynamic_extent || sizeof(T) <= EXTENT_V,
                  "Destination buffer is statically too small for T");

    constexpr auto byte_count = sizeof(T);
    ZTL_EXPECTS(byte_count <= ztl::size(buffer));

    const auto bytes = ztl::bit_cast<std::array<ztl::byte, byte_count>>(value);
    std::copy(std::cbegin(bytes), std::cend(bytes), std::begin(buffer));
    return byte_count;
}

template <typename T, std::size_t EXTENT_V, std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
inline constexpr auto read_from_buffer(T& value, ztl::span<const ztl::byte, EXTENT_V> buffer) noexcept -> std::size_t {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
    static_assert(EXTENT_V == ztl::dynamic_extent || sizeof(T) <= EXTENT_V,
                  "Source buffer is statically too small for T");

    constexpr auto byte_count = sizeof(T);
    ZTL_EXPECTS(byte_count <= ztl::size(buffer));

    std::array<ztl::byte, byte_count> bytes{};
    std::copy(std::cbegin(buffer), std::cbegin(buffer) + byte_count, std::begin(bytes));
    value = ztl::bit_cast<T>(bytes);
    return byte_count;
}

} // namespace details

template <ztl::endian ENDIAN_V = ztl::endian::little, typename T, std::size_t EXTENT_V>
inline constexpr auto serialize_to(T value, ztl::span<ztl::byte, EXTENT_V> buffer) noexcept -> std::size_t {
    return details::write_to_buffer(endian_swap<ztl::endian::native, ENDIAN_V>(value), buffer);
}

template <ztl::endian ENDIAN_V = ztl::endian::little, typename T, std::size_t EXTENT_V>
inline constexpr auto serialize_to(T val, ztl::span<ztl::byte, EXTENT_V> buffer, std::ptrdiff_t offset) noexcept
    -> std::size_t {
    return serialize_to<ENDIAN_V>(val, buffer.subspan(offset));
}

template <ztl::endian ENDIAN_V = ztl::endian::little, typename T, std::size_t EXTENT_V>
inline constexpr auto deserialize_from(T& value, ztl::span<const ztl::byte, EXTENT_V> buffer) noexcept -> std::size_t {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    using unsigned_type = std::make_unsigned_t<T>;
    unsigned_type tmp{};

    const auto size = details::read_from_buffer(tmp, buffer);
    tmp             = endian_swap<ENDIAN_V, ztl::endian::native>(tmp);

    value = static_cast<T>(tmp);
    return size;
}

template <ztl::endian ENDIAN_V = ztl::endian::little, typename T, std::size_t EXTENT_V>
inline constexpr auto deserialize_from(T& value, ztl::span<const ztl::byte, EXTENT_V> buffer,
                                       std::ptrdiff_t offset) noexcept -> std::size_t {
    return deserialize_from<ENDIAN_V>(value, buffer.subspan(offset));
}

} // namespace ztl

#endif // ZTL_SERIALIZE_HPP
