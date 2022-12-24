/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_SEQUENCE_SIZE_HPP
#define ZTL_META_SEQUENCE_SIZE_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/size.hpp"

#include "ztl/meta/constant/integral_constant.hpp"

namespace ztl::meta {

template <typename... ELEMENT_TS>
struct size<sequence<ELEMENT_TS...>> : integral_constant<unsigned, sizeof...(ELEMENT_TS)> {};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_SIZE_HPP
