// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UNITS_RATIO_HPP
#define ZTL_UNITS_RATIO_HPP

#include <cstdint>
#include <ratio>
#include <type_traits>

namespace ztl { namespace units {

template <typename T>
struct is_ratio : std::false_type {};
template <std::intmax_t N, std::intmax_t D>
struct is_ratio<std::ratio<N, D>> : std::true_type {};

namespace details {

// --- constexpr absolute value (intmax_t) -------------------------------------
constexpr std::intmax_t abs_intmax(std::intmax_t v) {
    return v < 0 ? -v : v;
}

// --- constexpr MSB position --------------------------------------------------
constexpr unsigned find_msb_impl(std::intmax_t v, unsigned n) {
    return v <= 0 ? n : find_msb_impl(v >> 1, n + 1);
}
constexpr unsigned find_msb(std::intmax_t v) {
    return find_msb_impl(abs_intmax(v), 0);
}

// --- ratio scaling (apply Num/Den to a value) --------------------------------
template <typename Ratio, typename T>
constexpr T ratio_scale(T val) {
    return static_cast<T>((static_cast<std::intmax_t>(val) * Ratio::num) / Ratio::den);
}

// --- ratio_bitsize: how many bits the larger component needs -----------------
template <typename Ratio>
struct ratio_bitsize {
    static constexpr unsigned num_bits = find_msb(Ratio::num);
    static constexpr unsigned den_bits = find_msb(Ratio::den);
    static constexpr unsigned value    = (num_bits > den_bits) ? num_bits : den_bits;
};

} // namespace details
}} // namespace ztl::units

#endif // ZTL_UNITS_RATIO_HPP
