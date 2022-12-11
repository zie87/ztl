/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_META_TYPE_TRAIT_IS_UNION_HPP
#define ZTL_META_TYPE_TRAIT_IS_UNION_HPP

#include "ztl/meta/constant/bool_constant.hpp"

namespace ztl::meta {

template <typename T>
struct is_union : bool_constant<__is_union(T)> {};

template <typename T>
inline constexpr bool is_union_v = is_union<T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_UNION_HPP
