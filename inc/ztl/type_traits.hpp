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

namespace ztl {

// NOTE: backport of C++17
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

// NOTE: backport of C++17
template <typename...>
struct make_void {
    using type = void;
};

template <typename... T>
using void_t = typename make_void<T...>::type;

// NOTE: backport of C++17; short-circuits, so later traits are only
// instantiated while all earlier ones are true.
template <typename...>
struct conjunction : std::true_type {};

template <typename B>
struct conjunction<B> : B {};

template <typename B, typename... BS>
struct conjunction<B, BS...> : std::conditional<static_cast<bool>(B::value), conjunction<BS...>, B>::type {};

// NOTE: backport of C++20
template <class T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <class T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;
} // namespace ztl

// NOTE: must stay below the definitions above; is_invocable.hpp includes this
// header for void_t, which only works because void_t is already defined here.
#include "ztl/type_traits/is_invocable.hpp"

#endif // ZTL_TYPETRAITS_HPP
