/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_META_SEQUENCE_PUSH_BACK_HPP
#define ZTL_META_SEQUENCE_PUSH_BACK_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/push_back.hpp"

namespace ztl::meta {

template <typename ELEMENT_T, typename... ITEM_TS>
struct push_back<sequence<ITEM_TS...>, ELEMENT_T> {
    using type = sequence<ITEM_TS..., ELEMENT_T>;
};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_PUSH_BACK_HPP
