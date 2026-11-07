// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_DETAILS_FUNCTIONSIGNATURE_HPP
#define ZTL_DETAILS_FUNCTIONSIGNATURE_HPP

#include "ztl/macros.hpp"
#include "ztl/type_traits.hpp"

namespace ztl { namespace details {

// Shared plumbing for the type-erased function wrappers (function_ref, inplace_function).

template <bool NOEXCEPT_V, typename R, typename F, typename... ARG_Ts>
struct invocable_selector : is_invocable_r<R, F, ARG_Ts...> {};

template <typename R, typename F, typename... ARG_Ts>
struct invocable_selector<true, R, F, ARG_Ts...> : is_nothrow_invocable_r<R, F, ARG_Ts...> {};

// Normalized form of a call signature. Wrappers are implemented once as a partial
// specialization on this tag, so `R(Args...)` and `R(Args...) noexcept` share code.
template <bool NOEXCEPT_V, typename R, typename... ARG_Ts>
struct function_signature {};

template <typename SIG_T>
struct signature_of; // intentionally undefined: SIG_T must be a function type

template <typename R, typename... ARG_Ts>
struct signature_of<R(ARG_Ts...)> {
    using type = function_signature<false, R, ARG_Ts...>;
};

// In standard C++14, noexcept is not part of the function type, so a distinct
// R(Args...) noexcept signature only exists with C++17 noexcept function types.
#if ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE
template <typename R, typename... ARG_Ts>
struct signature_of<R(ARG_Ts...) noexcept> {
    using type = function_signature<true, R, ARG_Ts...>;
};
#endif

template <typename SIG_T>
using signature_of_t = typename signature_of<SIG_T>::type;

}} // namespace ztl::details

#endif // ZTL_DETAILS_FUNCTIONSIGNATURE_HPP
