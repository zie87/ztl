/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_ITERATOR_DETAIL_ITERATOR_TAG_ZTL_HPP
#define ZTL_ITERATOR_DETAIL_ITERATOR_TAG_ZTL_HPP

namespace ztl {

// clang-format off
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : input_iterator_tag {};
struct bidirectional_iterator_tag : forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};
struct contiguous_iterator_tag : random_access_iterator_tag {};
// clang-format on

}  // namespace ztl

#endif  // ZTL_ITERATOR_DETAIL_ITERATOR_TAG_ZTL_HPP
