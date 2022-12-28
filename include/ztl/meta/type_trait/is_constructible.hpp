/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_TYPE_TRAIT_IS_CONSTRUCTIBLE_HPP
#define ZTL_META_TYPE_TRAIT_IS_CONSTRUCTIBLE_HPP

#include "ztl/meta/constant/bool_constant.hpp"

namespace ztl::meta {

namespace detail {

template <typename T, typename... ARG_Ts>
inline constexpr bool is_constructible_v = __is_constructible(T, ARG_Ts...);

template <typename T, typename... ARG_Ts>
inline constexpr bool is_nothrow_constructible_v = __is_nothrow_constructible(T, ARG_Ts...);

}  // namespace detail

template <typename T, typename... ARG_Ts>
struct is_constructible : ::ztl::meta::bool_constant<detail::is_constructible_v<T, ARG_Ts...>> {};

template <typename T, typename... ARG_Ts>
inline constexpr bool is_constructible_v = is_constructible<T, ARG_Ts...>::value;

template <typename T, typename... ARG_Ts>
struct is_nothrow_constructible : ::ztl::meta::bool_constant<detail::is_nothrow_constructible_v<T, ARG_Ts...>> {};

template <typename T, typename... ARG_Ts>
inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, ARG_Ts...>::value;
}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_CONSTRUCTIBLE_HPP
