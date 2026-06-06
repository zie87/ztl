// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_TYPETRAITS_ISINVOCABLE_HPP
#define ZTL_TYPETRAITS_ISINVOCABLE_HPP

#include "ztl/functional/invoke.hpp"

namespace ztl {
namespace details {

template <typename Void, typename R, typename F, typename... Args>
struct is_invocable_r_impl : std::false_type {};

template <typename R, typename F, typename... Args>
struct is_invocable_r_impl<::ztl::void_t<decltype(invoke(std::declval<F>(), std::declval<Args>()...))>, R, F, Args...>
    : std::integral_constant<
          bool, std::is_void<R>::value ||
                    std::is_convertible<decltype(invoke(std::declval<F>(), std::declval<Args>()...)), R>::value> {};

template <typename Void, typename R, typename F, typename... Args>
struct is_nothrow_invocable_r_impl : std::false_type {};

template <typename R, typename F, typename... Args>
struct is_nothrow_invocable_r_impl<::ztl::void_t<decltype(invoke(std::declval<F>(), std::declval<Args>()...))>, R, F,
                                   Args...>
    : std::integral_constant<
          bool, (std::is_void<R>::value ||
                 std::is_convertible<decltype(invoke(std::declval<F>(), std::declval<Args>()...)), R>::value) &&
                    noexcept(invoke(std::declval<F>(), std::declval<Args>()...))> {};

} // namespace details

template <typename R, typename F, typename... Args>
struct is_invocable_r : details::is_invocable_r_impl<void, R, F, Args...> {};

template <typename R, typename F, typename... Args>
struct is_nothrow_invocable_r : details::is_nothrow_invocable_r_impl<void, R, F, Args...> {};

} // namespace ztl

#endif // ZTL_TYPETRAITS_ISINVOCABLE_HPP
