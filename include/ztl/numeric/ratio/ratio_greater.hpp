#ifndef ZTL_NUMERIC_RATIO_RATIO_GREATER_HPP
#define ZTL_NUMERIC_RATIO_RATIO_GREATER_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/numeric/ratio/ratio_less.hpp"

namespace ztl {

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_greater : ztl::meta::bool_constant<ratio_less_v<RHS_V, LHS_V>> {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_greater_v = ratio_greater<LHS_V, RHS_V>::value;

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_greater_equal : ztl::meta::bool_constant<!ratio_less_v<LHS_V, RHS_V>> {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_greater_equal_v = ratio_greater_equal<LHS_V, RHS_V>::value;
}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_GREATER_HPP
