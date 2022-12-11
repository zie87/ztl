/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_META_TYPE_TRAIT_REMOVE_ALL_EXTENTS_HPP
#define ZTL_META_TYPE_TRAIT_REMOVE_ALL_EXTENTS_HPP

#include "ztl/types.hpp"

namespace ztl::meta {

template <typename T>
struct remove_all_extents {
    using type = T;
};

template <typename T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};

template <typename T, ztl::size_t N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};

template <typename T>
using remove_all_extents_t = typename remove_all_extents<T>::type;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_REMOVE_ALL_EXTENTS_HPP
