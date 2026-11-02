// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_NUMERIC_HPP
#define ZTL_NUMERIC_HPP

#include "ztl/assert.hpp"

#include <limits>
#include <type_traits>

namespace ztl {

namespace details {
// |x| for signed x; the caller guarantees x != numeric_limits<T>::min().
template <typename T>
constexpr T abs_val(T x) noexcept {
    ZTL_EXPECTS(!std::is_signed<T>::value || x != std::numeric_limits<T>::min());
    return x < T{0} ? static_cast<T>(-x) : x;
}

// |x| as the matching unsigned type; defined for every value, including min().
template <typename T>
constexpr typename std::make_unsigned<T>::type unsigned_abs(T x, std::true_type /*is_signed*/) noexcept {
    using unsigned_t = typename std::make_unsigned<T>::type;
    return x < T{0} ? static_cast<unsigned_t>(unsigned_t{0} - static_cast<unsigned_t>(x)) : static_cast<unsigned_t>(x);
}

template <typename T>
constexpr T unsigned_abs(T x, std::false_type /*is_signed*/) noexcept {
    return x;
}

template <typename T>
constexpr typename std::make_unsigned<T>::type unsigned_abs(T x) noexcept {
    return unsigned_abs(x, std::is_signed<T>{});
}

template <typename U>
constexpr U unsigned_gcd(U a, U b) noexcept {
    while (b != U{0}) {
        const U r = static_cast<U>(a % b);
        a         = b;
        b         = r;
    }
    return a;
}

template <typename M, typename N>
struct gcd_lcm_types {
    static_assert(std::is_integral<M>::value && std::is_integral<N>::value,
                  "ztl::gcd/ztl::lcm require integral arguments");
    static_assert(!std::is_same<typename std::remove_cv<M>::type, bool>::value &&
                      !std::is_same<typename std::remove_cv<N>::type, bool>::value,
                  "ztl::gcd/ztl::lcm do not accept bool");

    using common_t   = typename std::common_type<M, N>::type;
    using unsigned_t = typename std::make_unsigned<common_t>::type;

    static constexpr unsigned_t max = static_cast<unsigned_t>(std::numeric_limits<common_t>::max());
};
} // namespace details

// NOTE: like std::gcd/std::lcm (C++17), but computed on unsigned magnitudes so
// that no intermediate step can overflow (gcd(INT_MIN, 6) == 2). A result that
// does not fit the common type (gcd(INT_MIN, 0), or an lcm that is too large)
// violates a precondition: a compile error in constant expressions, a trap at run time.
template <typename M, typename N>
constexpr typename std::common_type<M, N>::type gcd(M m, N n) noexcept {
    using types = details::gcd_lcm_types<M, N>;
    using U     = typename types::unsigned_t;

    const U result =
        details::unsigned_gcd(static_cast<U>(details::unsigned_abs(m)), static_cast<U>(details::unsigned_abs(n)));
    ZTL_EXPECTS(result <= types::max);
    return static_cast<typename types::common_t>(result);
}

template <typename M, typename N>
constexpr typename std::common_type<M, N>::type lcm(M m, N n) noexcept {
    using types = details::gcd_lcm_types<M, N>;
    using U     = typename types::unsigned_t;

    const U a = static_cast<U>(details::unsigned_abs(m));
    const U b = static_cast<U>(details::unsigned_abs(n));
    if (a == U{0} || b == U{0}) {
        return 0;
    }

    const U reduced = static_cast<U>(a / details::unsigned_gcd(a, b));
    ZTL_EXPECTS(reduced <= types::max / b);
    return static_cast<typename types::common_t>(reduced * b);
}
} // namespace ztl

#endif // ZTL_NUMERIC_HPP
