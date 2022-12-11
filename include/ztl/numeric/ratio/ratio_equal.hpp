#ifndef ZTL_NUMERIC_RATIO_RATIO_EQUAL_HPP
#define ZTL_NUMERIC_RATIO_RATIO_EQUAL_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/ratio/ratio.hpp"

namespace ztl {

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_equal : ::ztl::meta::bool_constant<(LHS_V::num == RHS_V::num && LHS_V::den == RHS_V::den)> {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_equal_v = ratio_equal<LHS_V, RHS_V>::value;

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_not_equal : ::ztl::meta::bool_constant<!ratio_equal_v<LHS_V, RHS_V>> {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_not_equal_v = ratio_not_equal<LHS_V, RHS_V>::value;

}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_EQUAL_HPP
