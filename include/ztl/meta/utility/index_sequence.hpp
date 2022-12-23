/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_UTILITY_INDEX_SEQUENCE_HPP
#define ZTL_META_UTILITY_INDEX_SEQUENCE_HPP

#include "ztl/meta/utility/integer_sequence.hpp"
#include "ztl/types.hpp"

namespace ztl::meta {

template <ztl::size_t... INT_VS>
using index_sequence = ::ztl::meta::integer_sequence<ztl::size_t, INT_VS...>;

template <int CNT_V>
using make_index_sequence = ztl::meta::make_integer_sequence<ztl::size_t, CNT_V>;

}  // namespace ztl::meta

#endif  // ZTL_META_UTILITY_INDEX_SEQUENCE_HPP
