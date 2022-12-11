/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_NUMERIC_RATIO_RATIO_MULTIPLY_HPP
#define ZTL_NUMERIC_RATIO_RATIO_MULTIPLY_HPP

#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/detail/ratio/static_gcd.hpp"
#include "ztl/numeric/detail/safe_multiply.hpp"
#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

namespace detail {

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
class ratio_multiply {
private:
    static constexpr ztl::intmax_t gcd1 = detail::static_gcd_v<LHS_V::num, RHS_V::den>;
    static constexpr ztl::intmax_t gcd2 = detail::static_gcd_v<LHS_V::den, RHS_V::num>;

    static constexpr auto num = detail::safe_multiply_v<(LHS_V::num / gcd1), (RHS_V::num / gcd2)>;
    static constexpr auto den = detail::safe_multiply_v<(RHS_V::den / gcd1), (LHS_V::den / gcd2)>;

public:
    using type = ratio<num, den>;
};

}  // namespace detail

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
using ratio_multiply = typename detail::ratio_multiply<LHS_V, RHS_V>::type;
}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_MULTIPLY_HPP
