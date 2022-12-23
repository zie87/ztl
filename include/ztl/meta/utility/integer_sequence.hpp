/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_UTILITY_INTEGER_SEQUENCE_HPP
#define ZTL_META_UTILITY_INTEGER_SEQUENCE_HPP

#include "ztl/meta/concept/integral.hpp"
#include "ztl/types.hpp"

namespace ztl::meta {

template <meta::integral T, T... INT_VS>
struct integer_sequence {
    using value_type = T;
    template <T NEXT_V>
    using append = integer_sequence<T, INT_VS..., NEXT_V>;

    static constexpr auto size() noexcept -> ztl::size_t { return sizeof...(INT_VS); }
};

namespace detail {

template <typename T, int IDX_V>
struct make_integer_sequence {
    using type = typename make_integer_sequence<T, IDX_V - 1>::type::template append<IDX_V>;
};

template <typename T>
struct make_integer_sequence<T, -1> {
    using type = integer_sequence<T>;
};

}  // namespace detail

template <meta::integral T, int CNT_V>
using make_integer_sequence = typename detail::make_integer_sequence<T, CNT_V - 1>::type;
}  // namespace ztl::meta

#endif  // ZTL_META_UTILITY_INTEGER_SEQUENCE_HPP
