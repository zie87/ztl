/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_NUMERIC_DETAIL_SAFE_ADD_HPP
#define ZTL_NUMERIC_DETAIL_SAFE_ADD_HPP

#include "ztl/numeric/detail/ratio/static_sign.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {

// FIXME: redesign when limits is in place
// FIXME: could be generic and based on integral_constant

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V, ztl::intmax_t = detail::static_sign_v<RHS_V>>
struct safe_add;

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
struct safe_add<LHS_V, RHS_V, 0> {
    static constexpr ztl::intmax_t value = LHS_V;
};

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
struct safe_add<LHS_V, RHS_V, 1> {
private:
    static const ztl::intmax_t min = -INTMAX_MAX;
    static const ztl::intmax_t max = INTMAX_MAX;

    static_assert((LHS_V <= (max - RHS_V)), "overflow in add");

public:
    static constexpr ztl::intmax_t value = LHS_V + RHS_V;
};

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
struct safe_add<LHS_V, RHS_V, -1> {
private:
    static const ztl::intmax_t min = -INTMAX_MAX;
    static const ztl::intmax_t max = INTMAX_MAX;

    static_assert(((min - RHS_V) <= LHS_V), "overflow in add");

public:
    static constexpr ztl::intmax_t value = LHS_V + RHS_V;
};

template <ztl::intmax_t LHS_V, ztl::intmax_t RHS_V>
inline constexpr ztl::intmax_t safe_add_v = safe_add<LHS_V, RHS_V>::value;

}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_SAFE_ADD_HPP
