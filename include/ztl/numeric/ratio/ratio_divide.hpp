/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_NUMERIC_RATIO_RATIO_DIVIDE_HPP
#define ZTL_NUMERIC_RATIO_RATIO_DIVIDE_HPP

#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/detail/ratio/static_gcd.hpp"
#include "ztl/numeric/detail/safe_multiply.hpp"
#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

namespace detail {

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_divide {
private:
    static constexpr ztl::intmax_t gcd_num = detail::static_gcd_v<LHS_V::num, RHS_V::num>;
    static constexpr ztl::intmax_t gcd_den = detail::static_gcd_v<LHS_V::den, RHS_V::den>;

    static constexpr auto num = detail::safe_multiply_v<(LHS_V::num / gcd_num), (RHS_V::den / gcd_den)>;
    static constexpr auto den = detail::safe_multiply_v<(RHS_V::num / gcd_num), (LHS_V::den / gcd_den)>;

public:
    using type = ratio<num, den>;
};
}  // namespace detail

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
using ratio_divide = typename detail::ratio_divide<LHS_V, RHS_V>::type;
}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_DIVIDE_HPP
