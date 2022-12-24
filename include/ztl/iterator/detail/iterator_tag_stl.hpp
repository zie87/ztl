/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_ITERATOR_DETAIL_ITERATOR_TAG_STL_HPP
#define ZTL_ITERATOR_DETAIL_ITERATOR_TAG_STL_HPP

#include <iterator>

namespace ztl {

// clang-format off
using std::input_iterator_tag; 
using std::output_iterator_tag;
using std::forward_iterator_tag;
using std::bidirectional_iterator_tag;
using std::random_access_iterator_tag;
using std::contiguous_iterator_tag;
// clang-format on

}  // namespace ztl

#endif  // ZTL_ITERATOR_DETAIL_ITERATOR_TAG_STL_HPP
