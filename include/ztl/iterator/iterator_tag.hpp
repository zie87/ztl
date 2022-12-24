/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_ITERATOR_ITERATOR_TAG_HPP
#define ZTL_ITERATOR_ITERATOR_TAG_HPP

#ifdef ZTL_STL_COMPATIBLE_ITERATORS
#include "ztl/iterator/detail/iterator_tag_stl.hpp"
#else
#include "ztl/iterator/detail/iterator_tag_ztl.hpp"
#endif  // ZTL_STL_COMPATIBLE_ITERATORS

#endif  // ZTL_ITERATOR_ITERATOR_TAG_HPP
