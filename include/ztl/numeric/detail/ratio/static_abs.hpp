/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_NUMERIC_DETAIL_RATIO_STATIC_ABS_HPP
#define ZTL_NUMERIC_DETAIL_RATIO_STATIC_ABS_HPP

#include "ztl/meta/constant/integral_constant.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {

consteval auto static_abs_help(::ztl::intmax_t val) noexcept { return (val < 0) ? -val : val; }

template <::ztl::intmax_t V>
struct static_abs : ::ztl::meta::integral_constant<::ztl::intmax_t, static_abs_help(V)> {};

template <::ztl::intmax_t V>
inline constexpr ::ztl::intmax_t static_abs_v = static_abs<V>::value;
}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_RATIO_STATIC_ABS_HPP
