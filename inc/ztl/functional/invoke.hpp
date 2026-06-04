// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_FUNCTIONAL_INVOKE_HPP
#define ZTL_FUNCTIONAL_INVOKE_HPP

#include <type_traits>
#include <utility>

namespace ztl
{
namespace details
{

// -----------------------------------------------------------------------------
// Minimal C++14 std::invoke replacement.
// Reference_wrapper support is intentionally omitted; function_ref should bind
// directly to the object whose lifetime is managed by the caller.
// -----------------------------------------------------------------------------
template<typename F, typename T, typename... Args>
auto invoke_memfun(F&& f,
                   T*  t,
                   Args&&... args) noexcept(noexcept((t->*f)(std::forward<Args>(args)...)))
    -> decltype((t->*f)(std::forward<Args>(args)...))
{
    return (t->*f)(std::forward<Args>(args)...);
}

template<typename F, typename T, typename... Args>
auto invoke_memfun(F&& f, T&& t, Args&&... args) noexcept(
    noexcept((std::forward<T>(t).*f)(std::forward<Args>(args)...)))
    -> decltype((std::forward<T>(t).*f)(std::forward<Args>(args)...))
{
    return (std::forward<T>(t).*f)(std::forward<Args>(args)...);
}

template<typename F, typename T>
auto invoke_memobj(F&& f, T* t) noexcept(noexcept(t->*f)) -> decltype(t->*f)
{
    return t->*f;
}

template<typename F, typename T>
auto invoke_memobj(F&& f, T&& t) noexcept(noexcept(std::forward<T>(t).*f))
    -> decltype(std::forward<T>(t).*f)
{
    return std::forward<T>(t).*f;
}

} // namespace details

template<
    typename F,
    typename T,
    typename... Args,
    typename std::enable_if<std::is_member_function_pointer<typename std::decay<F>::type>::value,
                            int>::type
    = 0>
auto invoke(F&& f, T&& t, Args&&... args) noexcept(noexcept(
    details::invoke_memfun(std::forward<F>(f), std::forward<T>(t), std::forward<Args>(args)...)))
    -> decltype(details::invoke_memfun(std::forward<F>(f),
                                       std::forward<T>(t),
                                       std::forward<Args>(args)...))
{
    return details::invoke_memfun(std::forward<F>(f),
                                  std::forward<T>(t),
                                  std::forward<Args>(args)...);
}

template<typename F,
         typename T,
         typename std::enable_if<std::is_member_object_pointer<typename std::decay<F>::type>::value,
                                 int>::type
         = 0>
auto invoke(F&& f, T&& t) noexcept(noexcept(details::invoke_memobj(std::forward<F>(f),
                                                                   std::forward<T>(t))))
    -> decltype(details::invoke_memobj(std::forward<F>(f), std::forward<T>(t)))
{
    return details::invoke_memobj(std::forward<F>(f), std::forward<T>(t));
}

template<
    typename F,
    typename... Args,
    typename std::enable_if<!std::is_member_pointer<typename std::decay<F>::type>::value, int>::type
    = 0>
auto invoke(F&& f,
            Args&&... args) noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
    -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
{
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

template<typename R, typename F, typename... Args>
typename std::enable_if<!std::is_void<R>::value, R>::type invoke_r(F&& f, Args&&... args)
{
    return invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename R, typename F, typename... Args>
typename std::enable_if<std::is_void<R>::value, void>::type invoke_r(F&& f, Args&&... args)
{
    invoke(std::forward<F>(f), std::forward<Args>(args)...);
}
} // namespace ztl

#endif // ZTL_FUNCTIONAL_INVOKE_HPP
