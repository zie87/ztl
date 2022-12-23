/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_MAP_HPP
#define ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_MAP_HPP

#include "ztl/tuple/detail/tuple_element.hpp"
#include "ztl/meta/utility/index_sequence.hpp"

namespace ztl::detail {

template <typename... ELEMENT_TS>
struct tuple_element_map : ELEMENT_TS... {
    using ELEMENT_TS::operator[]...;
};

template <typename, typename...>
struct make_tuple_map;

template <ztl::size_t... IDX_VS, typename... ELEMENT_TS>
struct make_tuple_map<ztl::meta::index_sequence<IDX_VS...>, ELEMENT_TS...> {
    using type = tuple_element_map<tuple_element<IDX_VS, ELEMENT_TS>...>;
};

template <typename... ELEMENT_TS>
using tuple_map_t = typename make_tuple_map<ztl::meta::make_index_sequence<sizeof...(ELEMENT_TS)>, ELEMENT_TS...>::type;

}  // namespace ztl::detail

#endif  // ZTL_TUPLE_DETAIL_TUPLE_ELEMENT_MAP_HPP
