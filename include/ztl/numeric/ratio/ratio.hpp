/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_NUMERIC_RATIO_RATIO_HPP
#define ZTL_NUMERIC_RATIO_RATIO_HPP

#include "ztl/numeric/detail/ratio/static_abs.hpp"
#include "ztl/numeric/detail/ratio/static_gcd.hpp"
#include "ztl/numeric/detail/ratio/static_sign.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

template <::ztl::intmax_t NUM_V, ztl::intmax_t DENOM_V = 1>
class ratio {
private:
    static_assert(DENOM_V != 0, "ratio divide by 0");
    static_assert(-INTMAX_MAX <= NUM_V, "ratio numerator is out of range");
    static_assert(-INTMAX_MAX <= DENOM_V, "ratio denominator is out of range");

    static constexpr ::ztl::intmax_t sign    = detail::static_sign_v<NUM_V> * detail::static_sign_v<DENOM_V>;
    static constexpr ::ztl::intmax_t num_abs = detail::static_abs_v<NUM_V>;
    static constexpr ::ztl::intmax_t den_abs = detail::static_abs_v<DENOM_V>;
    static constexpr ::ztl::intmax_t gcd     = detail::static_gcd_v<num_abs, den_abs>;

public:
    static constexpr ::ztl::intmax_t num = sign * num_abs / gcd;
    static constexpr ::ztl::intmax_t den = den_abs / gcd;

    using type = ratio<num, den>;
};

}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_HPP
