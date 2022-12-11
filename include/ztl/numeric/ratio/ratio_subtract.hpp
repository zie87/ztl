#ifndef ZTL_NUMERIC_RATIO_RATIO_SUBTRACT_HPP
#define ZTL_NUMERIC_RATIO_RATIO_SUBTRACT_HPP

#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/detail/ratio/static_gcd.hpp"
#include "ztl/numeric/detail/safe_multiply.hpp"
#include "ztl/numeric/detail/safe_substract.hpp"
#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/numeric/ratio/ratio_multiply.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

namespace detail {

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
class ratio_subtract {
private:
    static constexpr ztl::intmax_t gcd_num = detail::static_gcd_v<LHS_V::num, RHS_V::num>;
    static constexpr ztl::intmax_t gcd_den = detail::static_gcd_v<LHS_V::den, RHS_V::den>;

    static constexpr auto lhs_norm = detail::safe_multiply_v<(LHS_V::num / gcd_num), (RHS_V::den / gcd_den)>;
    static constexpr auto rhs_norm = detail::safe_multiply_v<(RHS_V::num / gcd_num), (LHS_V::den / gcd_den)>;

    using gcd_type                     = ztl::ratio<gcd_num, (LHS_V::den / gcd_den)>;
    static constexpr ztl::intmax_t ret = detail::safe_substract_v<lhs_norm, rhs_norm>;

public:
    using type = ::ztl::ratio_multiply<gcd_type, ztl::ratio<ret, RHS_V::den>>;
};

template <detail::ratio_concept RATIO_T, ztl::intmax_t DENOM_V>
struct ratio_subtract<RATIO_T, ztl::ratio<0, DENOM_V>> {
    using type = RATIO_T;
};

}  // namespace detail

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
using ratio_subtract = typename detail::ratio_subtract<LHS_V, RHS_V>::type;
}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_SUBTRACT_HPP
