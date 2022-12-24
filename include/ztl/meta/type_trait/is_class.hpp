/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_TYPE_TRAIT_IS_CLASS_HPP
#define ZTL_META_TYPE_TRAIT_IS_CLASS_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/meta/type_trait/is_union.hpp"

// is based on: https://en.cppreference.com/w/cpp/types/is_class
namespace ztl::meta {

namespace detail {
template <typename T>
auto test_is_class(int T::*) -> bool_constant<!is_union_v<T>>;

template <typename>
auto test_is_class(...) -> false_type;
}  // namespace detail

template <class T>
struct is_class : decltype(detail::test_is_class<T>(nullptr)) {};

template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_CLASS_HPP
