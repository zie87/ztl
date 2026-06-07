// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_NARROW_HPP
#define ZTL_NARROW_HPP

#include "ztl/assert.hpp"

#include <type_traits>
#include <utility>

namespace ztl {

// ============================================================================
//  narrow_cast — unchecked narrowing (equivalent to GSL narrow_cast)
// ============================================================================
//  A searchable marker for intentional narrowing conversions.
//  No runtime cost, no safety check — just a static_cast.

template <class T, class U>
constexpr T narrow_cast(U&& u) noexcept {
    return static_cast<T>(std::forward<U>(u));
}

// ============================================================================
//  to_underlying — enum to its underlying integer type (C++23 backport)
// ============================================================================

template <class T>
constexpr typename std::underlying_type<T>::type to_underlying(T e) noexcept {
    using type = typename std::underlying_type<T>::type;
    return static_cast<type>(e);
}

// ============================================================================
//  narrow — checked narrowing conversion (equivalent to GSL narrow)
// ============================================================================
//
//  Behaviour:
//    constexpr context  →  compile-time error if value is not preserved
//    runtime            →  trap_forever() if value is not preserved
//
//  The trick: trap_forever() is NOT constexpr.  When the narrowing check
//  fails during constant evaluation the compiler tries to evaluate the
//  trap_forever() branch, which is not a constant expression → hard error.
//  At runtime the same branch simply traps.
//
//  Checks performed (same as GSL):
//    1. Round-trip: static_cast<U>(static_cast<T>(u)) == u
//    2. Sign preservation (only when T and U differ in signedness):
//       the sign bit must not flip.
//
//  Example:
//    constexpr int a = ztl::narrow<int>(42ULL);           // OK
//    constexpr int b = ztl::narrow<int>(5000000000ULL);   // compile error
//    int c = ztl::narrow<int>(runtime_value);             // traps if lossy

namespace details {

// Detect whether a cast changed the sign of a value.
// Only meaningful when T and U differ in signedness; guarded below.
// Each comparison is same-type, so no signed/unsigned warnings.
template <class T, class U>
constexpr bool narrow_sign_changed(T t, U u) noexcept {
    return (t < T{}) != (u < U{});
}

// The check itself, factored out so `narrow` stays a clean single return.
// Returns true when the narrowing is SAFE (value preserved).
template <class T, class U>
constexpr bool narrow_is_safe(U u) noexcept {
    return static_cast<U>(narrow_cast<T>(u)) == u &&
           !(std::is_signed<T>::value != std::is_signed<U>::value && narrow_sign_changed(narrow_cast<T>(u), u));
}

} // namespace details

template <class T, class U>
constexpr T narrow(U u) noexcept {
    ZTL_EXPECTS(details::narrow_is_safe<T>(u));
    return narrow_cast<T>(u);
}

} // namespace ztl

#endif // ZTL_NARROW_HPP
