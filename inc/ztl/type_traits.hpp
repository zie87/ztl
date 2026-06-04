// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_TYPETRAITS_HPP
#define ZTL_TYPETRAITS_HPP

#include <type_traits>

namespace ztl
{
  
// NOTE: backport of C++17
template<typename...>
using void_t = void;

// NOTE: backport of C++20
template<class T>
using remove_cv_t = typename std::remove_cv<T>::type;

template<class T>
struct remove_cvref
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;
} // namespace ztl

#endif // ZTL_TYPETRAITS_HPP
