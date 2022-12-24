/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_DETAIL_SEQUENCE_EMPTY_HPP
#define ZTL_META_DETAIL_SEQUENCE_EMPTY_HPP

namespace ztl::meta {

template <typename>
struct empty;

template <typename LIST_T>
inline constexpr bool empty_v = empty<LIST_T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_DETAIL_SEQUENCE_EMPTY_HPP
