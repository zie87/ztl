/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_SEQUENCE_EMPTY_HPP
#define ZTL_META_SEQUENCE_EMPTY_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/empty.hpp"

#include "ztl/meta/constant/bool_constant.hpp"

namespace ztl::meta {

template <>
struct empty<sequence<>> : true_type {};

template <typename... ELEMENT_TS>
struct empty<sequence<ELEMENT_TS...>> : false_type {};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_EMPTY_HPP
