/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP
#define ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP

#include "ztl/meta/detail/constant/type_constant.hpp"
#include "ztl/meta/concept/integral.hpp"

namespace ztl::meta {

template <integral T, T V>
struct integral_constant : detail::type_constant<T, V> {};

}  // namespace ztl::meta

#endif  // ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP
