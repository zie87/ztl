// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_ITERATORTAGS_HPP
#define ZTL_ITERATORTAGS_HPP

#include <iterator>

namespace ztl {

// ===========================================================================
// contiguous_iterator_tag
//
// C++14 has no std::contiguous_iterator_tag.  We provide one (refining
// random-access) so an iterator can advertise contiguity the way C++20 expects,
// via `using iterator_concept = ztl::contiguous_iterator_tag;` or by passing it
// as the IteratorConcept argument of iterator_interface.  The interface
// normalizes the operation set to random-access and keeps the std
// iterator_category at random_access_iterator_tag, while iterator_concept
// carries the contiguous tag.  ztl::is_contiguous_iterator keys off it.
// ===========================================================================
struct contiguous_iterator_tag : std::random_access_iterator_tag {};

} // namespace ztl

#endif // ZTL_ITERATORTAGS_HPP
