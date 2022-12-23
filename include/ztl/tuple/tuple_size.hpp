/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_TUPLE_SIZE_HPP
#define ZTL_TUPLE_TUPLE_SIZE_HPP

#include "ztl/tuple/detail/tuple_concept.hpp"
#include "ztl/meta/constant/integral_constant.hpp"
#include "ztl/tuple/tuple_base.hpp"

namespace ztl {

template <detail::tuple_concept TUPLE_T>
struct tuple_size : ztl::meta::integral_constant<ztl::size_t, TUPLE_T::size()> {};

template <detail::tuple_concept TUPLE_T>
inline constexpr auto tuple_size_v = tuple_size<TUPLE_T>::value;

}  // namespace ztl

#endif  // ZTL_TUPLE_TUPLE_SIZE_HPP
