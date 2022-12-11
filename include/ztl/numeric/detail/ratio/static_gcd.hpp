#ifndef ZTL_NUMERIC_DETAIL_RATIO_STATIC_GCD_HPP
#define ZTL_NUMERIC_DETAIL_RATIO_STATIC_GCD_HPP

#include "ztl/meta/constant/integral_constant.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {
template <::ztl::intmax_t X_V, ::ztl::intmax_t Y_V>
struct static_gcd : static_gcd<Y_V, (X_V % Y_V)> {};

template <::ztl::intmax_t V>
struct static_gcd<V, 0> : ::ztl::meta::integral_constant<::ztl::intmax_t, V> {};

template <>
struct static_gcd<0, 0> : ::ztl::meta::integral_constant<::ztl::intmax_t, 1> {};

template <::ztl::intmax_t X_V, ::ztl::intmax_t Y_V>
inline constexpr ztl::intmax_t static_gcd_v = static_gcd<X_V, Y_V>::value;

}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_RATIO_STATIC_GCD_HPP
