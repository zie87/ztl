/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_NUMERIC_DETAIL_SAFE_MULTIPLY_HPP
#define ZTL_NUMERIC_DETAIL_SAFE_MULTIPLY_HPP

#include "ztl/numeric/detail/ratio/static_abs.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {

// FIXME: redesign when limits is in place
// FIXME: could be generic and based on integral_constant

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
class safe_multiply {
    static constexpr ztl::intmax_t nan = (1LL << (sizeof(intmax_t) * 8 - 1));
    static constexpr ztl::intmax_t min = nan + 1;
    static constexpr ztl::intmax_t max = -min;
    static constexpr ztl::intmax_t x   = detail::static_abs_v<LHS_V>;
    static constexpr ztl::intmax_t y   = detail::static_abs_v<RHS_V>;

    static_assert(LHS_V != nan && RHS_V != nan && x <= max / y, "overflow in multiply");

public:
    static constexpr ztl::intmax_t value = LHS_V * RHS_V;
};

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
inline constexpr auto safe_multiply_v = safe_multiply<LHS_V, RHS_V>::value;
}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_SAFE_MULTIPLY_HPP
