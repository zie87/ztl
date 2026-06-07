// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_NUMERIC_HPP
#define ZTL_NUMERIC_HPP

#include <numeric>

namespace ztl {

namespace details {
template <typename T>
constexpr T abs_val(T x) {
    return x < 0 ? -x : x;
}

template <typename M, typename N>
constexpr std::common_type_t<M, N> gcd_impl(M m, N n) {
    return n == 0 ? m : gcd_impl(n, m % n); // recursive for C++11 constexpr
}
} // namespace details

template <typename M, typename N>
constexpr std::common_type_t<M, N> gcd(M m, N n) {
    return details::gcd_impl(details::abs_val(m), details::abs_val(n));
}

template <class M, class N>
constexpr std::common_type_t<M, N> lcm(M m, N n) {
    return (m == 0 || n == 0) ? 0 : details::abs_val(m) / gcd(m, n) * details::abs_val(n);
}
} // namespace ztl

#endif // ZTL_NUMERIC_HPP
