/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_TUPLE_ELEMENT_HPP
#define ZTL_TUPLE_TUPLE_ELEMENT_HPP

#include "ztl/tuple/detail/tuple_concept.hpp"
#include "ztl/tuple/tuple_base.hpp"
#include "ztl/types.hpp"

namespace ztl {

namespace detail {
template <ztl::size_t, typename>
struct get_tuple_element_type;

template <ztl::size_t IDX_V, typename HEAD_T, typename... TAIL_TS>
struct get_tuple_element_type<IDX_V, ztl::tuple<HEAD_T, TAIL_TS...>>
    : get_tuple_element_type<IDX_V - 1, ztl::tuple<TAIL_TS...>> {};

template <typename HEAD_T, typename... TAIL_TS>
struct get_tuple_element_type<0, ztl::tuple<HEAD_T, TAIL_TS...>> {
    using type = HEAD_T;
};
}  // namespace detail

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
struct tuple_element {
    static_assert(IDX_V <= TUPLE_T::size(), "tuple index out of bounds");
    using type = typename detail::get_tuple_element_type<IDX_V, TUPLE_T>::type;
};

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
using tuple_element_t = typename tuple_element<IDX_V, TUPLE_T>::type;

}  // namespace ztl

#endif  // ZTL_TUPLE_TUPLE_ELEMENT_HPP
