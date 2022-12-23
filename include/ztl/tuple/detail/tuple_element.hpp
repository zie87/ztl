/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_HPP
#define ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_HPP

#include "ztl/types.hpp"
#include "ztl/meta/constant/integral_constant.hpp"

namespace ztl::detail {

template <ztl::size_t IDX_V>
using index_type = ztl::meta::integral_constant<ztl::size_t, IDX_V>;

template <ztl::size_t IDX_V, typename T>
struct tuple_element {
    using type = T;

    constexpr auto operator[](index_type<IDX_V>) & -> decltype(auto) { return (value); }
    constexpr auto operator[](index_type<IDX_V>) const& -> decltype(auto) { return (value); }
    constexpr auto operator[](index_type<IDX_V>) && -> decltype(auto) {
        return (static_cast<tuple_element&&>(*this).value);
    }

    [[no_unique_address]] type value;
};

}  // namespace ztl::detail

#endif  // ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_HPP
