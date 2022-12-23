/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_DETAIL_IS_TUPLE_HPP
#define ZTL_TUPLE_DETAIL_IS_TUPLE_HPP

#include "ztl/meta/constant/bool_constant.hpp"

#include "ztl/tuple/tuple_base.hpp"

namespace ztl::detail {

template <typename T>
struct is_tuple : ztl::meta::false_type {};

template <typename... TS>
struct is_tuple<ztl::tuple<TS...>> : ztl::meta::true_type {};

template <typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

}  // namespace ztl::detail

#endif  // ZTL_TUPLE_DETAIL_IS_TUPLE_HPP
