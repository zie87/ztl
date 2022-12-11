/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP
#define ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP

namespace ztl::meta::detail {

template <typename T>
inline constexpr bool always_false = false;

}  // namespace ztl::meta::detail

#endif  // ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP
