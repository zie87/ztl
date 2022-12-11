/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_UTILITY_ENDIAN_ENDIAN_SWAP_HPP
#define ZTL_UTILITY_ENDIAN_ENDIAN_SWAP_HPP

#include "ztl/meta/concept/integral.hpp"
#include "ztl/meta/type_trait/has_unique_object_representations.hpp"
#include "ztl/utility/endian/byteswap.hpp"
#include "ztl/utility/endian/endian.hpp"

namespace ztl {

namespace detail {

template <::ztl::endian FROM_V, ::ztl::endian TO_V>
struct endian_swapper {
    template <typename T>
    static inline constexpr auto swap(T val) -> T {
        return ::ztl::byteswap(val);
    }
};

template <::ztl::endian ENDIAN_V>
struct endian_swapper<ENDIAN_V, ENDIAN_V> {
    template <typename T>
    static inline constexpr auto swap(T val) -> T {
        return val;
    }
};

}  // namespace detail

template <::ztl::endian FROM_V, ::ztl::endian TO_V, ::ztl::meta::integral T>
requires meta::has_unique_object_representations_v<T>
[[nodiscard]] constexpr auto endian_swap(T val) noexcept -> T {
    return detail::endian_swapper<FROM_V, TO_V>::swap(val);
}

template <::ztl::endian TO_V, ::ztl::meta::integral T>
requires meta::has_unique_object_representations_v<T>
[[nodiscard]] constexpr auto to_endian(T val) noexcept -> T { return endian_swap<endian::native, TO_V, T>(val); }

template <::ztl::endian FROM_V, ::ztl::meta::integral T>
requires meta::has_unique_object_representations_v<T>
[[nodiscard]] constexpr auto from_endian(T val) noexcept -> T { return endian_swap<FROM_V, endian::native, T>(val); }

}  // namespace ztl

#endif  // ZTL_UTILITY_ENDIAN_ENDIAN_SWAP_HPP
