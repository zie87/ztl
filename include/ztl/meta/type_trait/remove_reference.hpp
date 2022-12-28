/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_TYPE_TRAIT_REMOVE_REFERENCE_HPP
#define ZTL_META_TYPE_TRAIT_REMOVE_REFERENCE_HPP

namespace ztl::meta {
// clang-format off
template< typename T > struct remove_reference      { using type = T; };
template< typename T > struct remove_reference<T&>  { using type = T; };
template< typename T > struct remove_reference<T&&> { using type = T; };
// clang-format on

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;
}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_REMOVE_REFERENCE_HPP
