/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_TYPE_TRAIT_IS_REFERENCE_HPP
#define ZTL_META_TYPE_TRAIT_IS_REFERENCE_HPP

#include <ztl/meta/constant/bool_constant.hpp>

namespace ztl::meta {
// clang-format off
template<typename T> struct is_lvalue_reference     : meta::false_type {};
template<typename T> struct is_lvalue_reference<T&> : meta::true_type {};
// clang-format on

template <typename T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;
}  // namespace ztl::meta

namespace ztl::meta {
// clang-format off
template<typename T> struct is_rvalue_reference      : meta::false_type {};
template<typename T> struct is_rvalue_reference<T&&> : meta::true_type {};
// clang-format on

template <typename T>
inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;
}  // namespace ztl::meta

namespace ztl::meta {
// clang-format off
template<typename T> struct is_reference : meta::bool_constant< is_rvalue_reference_v<T> || is_lvalue_reference_v<T> > {};
// clang-format on

template <typename T>
inline constexpr bool is_reference_v = is_reference<T>::value;
}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_REFERENCE_HPP
