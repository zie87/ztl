#ifndef ZTL_ITERATOR_ITERATOR_TAG_HPP
#define ZTL_ITERATOR_ITERATOR_TAG_HPP

#ifdef ZTL_STL_COMPATIBLE_ITERATORS
#include "ztl/iterator/detail/iterator_tag_stl.hpp"
#else
#include "ztl/iterator/detail/iterator_tag_ztl.hpp"
#endif  // ZTL_STL_COMPATIBLE_ITERATORS

#endif  // ZTL_ITERATOR_ITERATOR_TAG_HPP
