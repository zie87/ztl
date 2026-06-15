// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_MEMORY_HPP
#define ZTL_MEMORY_HPP

#include <memory>
#include <type_traits>

namespace ztl {

namespace details {

// clang-format off
template<int N>
struct priority_tag : priority_tag<N - 1> {};

template<>
struct priority_tag<0> {};
// clang-format on

template <class T>
constexpr T* to_address_impl(T* p, priority_tag<2>) noexcept {
    static_assert(!std::is_function<T>::value, "to_address() does not accept function pointers");
    return p;
}

template <class Ptr>
constexpr auto to_address_impl(const Ptr& p, priority_tag<1>) noexcept
    -> decltype(std::pointer_traits<Ptr>::to_address(p)) {
    return std::pointer_traits<Ptr>::to_address(p);
}

template <class Ptr>
constexpr auto to_address_impl(const Ptr& p, priority_tag<0>) noexcept
    -> decltype(to_address_impl(p.operator->(), priority_tag<2>{})) {
    return to_address_impl(p.operator->(), priority_tag<2>{});
}

} // namespace details

template <class T>
constexpr T* to_address(T* p) noexcept {
    return details::to_address_impl(p, details::priority_tag<2>{});
}

template <class Ptr>
constexpr auto to_address(const Ptr& p) noexcept -> decltype(details::to_address_impl(p, details::priority_tag<2>{})) {
    return details::to_address_impl(p, details::priority_tag<2>{});
}

// std::addressof is constexpr (since C++17) and noexcept; we match that here.
// The reinterpret_cast idiom cannot appear in a constant expression, so we use
// the compiler builtin when one is available (it is on GCC, Clang and IAR) and
// only fall back to the cast otherwise. The fallback is still noexcept; it just
// is not usable in a constant expression.
#if defined(__has_builtin)
#if __has_builtin(__builtin_addressof)
#define ZTL_HAS_BUILTIN_ADDRESSOF 1
#endif
#endif

#ifdef ZTL_HAS_BUILTIN_ADDRESSOF
template <typename T>
constexpr T* addressof(T& value) noexcept {
    return __builtin_addressof(value);
}
#else
template <typename T>
T* addressof(T& value) noexcept {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<char const volatile&>(value)));
}
#endif

} // namespace ztl

#endif // ZTL_MEMORY_HPP
