/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_DETAIL_SEQUENCE_SIZE_HPP
#define ZTL_META_DETAIL_SEQUENCE_SIZE_HPP

namespace ztl::meta {

template <typename>
struct size;

template <typename LIST_T>
inline constexpr auto size_v = size<LIST_T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_DETAIL_SEQUENCE_SIZE_HPP
