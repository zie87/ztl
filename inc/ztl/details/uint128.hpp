// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_DETAILS_UINT128_HPP
#define ZTL_DETAILS_UINT128_HPP

#include <cstdint>

namespace ztl { namespace details {

// NOTE: minimal portable 128-bit unsigned arithmetic (C++14 constexpr). Not every
// supported compiler offers __int128 (e.g. IAR), so it is built from two halves.
// Only the operations needed for exact multiply-then-divide are provided.
struct uint128 {
    std::uint64_t hi;
    std::uint64_t lo;
};

constexpr uint128 uint128_mul(std::uint64_t x, std::uint64_t y) noexcept {
    const std::uint64_t mask = 0xFFFFFFFFu;
    const std::uint64_t x0   = x & mask;
    const std::uint64_t x1   = x >> 32;
    const std::uint64_t y0   = y & mask;
    const std::uint64_t y1   = y >> 32;

    const std::uint64_t p00 = x0 * y0;
    const std::uint64_t p01 = x0 * y1;
    const std::uint64_t p10 = x1 * y0;
    const std::uint64_t p11 = x1 * y1;

    const std::uint64_t mid = (p00 >> 32) + (p01 & mask) + (p10 & mask);
    return uint128{p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32), (p00 & mask) | (mid << 32)};
}

constexpr uint128 uint128_add(uint128 a, std::uint64_t b) noexcept {
    const std::uint64_t lo = a.lo + b;
    return uint128{a.hi + (lo < a.lo ? 1u : 0u), lo};
}

constexpr uint128 uint128_sub(uint128 a, std::uint64_t b) noexcept {
    return uint128{a.hi - (a.lo < b ? 1u : 0u), a.lo - b};
}

constexpr bool uint128_less(uint128 a, std::uint64_t b) noexcept {
    return a.hi == 0u && a.lo < b;
}

// Truncating division; the caller guarantees d != 0.
constexpr uint128 uint128_div(uint128 n, std::uint64_t d) noexcept {
    uint128       q{n.hi / d, 0u};
    std::uint64_t r = n.hi % d;

    for (int bit = 63; bit >= 0; --bit) {
        const bool carry = (r >> 63) != 0u;
        r                = (r << 1) | ((n.lo >> bit) & 1u);
        // r < d held before the shift, so (carry:r) < 2d and one subtraction suffices
        if (carry || r >= d) {
            r -= d;
            q.lo |= std::uint64_t{1} << bit;
        }
    }
    return q;
}

}} // namespace ztl::details

#endif // ZTL_DETAILS_UINT128_HPP
