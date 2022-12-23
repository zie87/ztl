/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_DETAIL_TUPLE_CONCEPT_HPP
#define ZTL_TUPLE_DETAIL_TUPLE_CONCEPT_HPP

#include "ztl/tuple/detail/is_tuple.hpp"

namespace ztl::detail {

template <typename T>
concept tuple_concept = is_tuple_v<T>;

}  // namespace ztl::detail

#endif  // ZTL_TUPLE_DETAIL_TUPLE_CONCEPT_HPP
