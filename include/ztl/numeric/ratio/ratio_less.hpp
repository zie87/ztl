#ifndef ZTL_NUMERIC_RATIO_RATIO_LESS_HPP
#define ZTL_NUMERIC_RATIO_RATIO_LESS_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/numeric/detail/ratio/ratio_concept.hpp"
#include "ztl/numeric/detail/ratio/static_sign.hpp"
#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/numeric/ratio/ratio_equal.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

namespace detail {
// based on: boost::ratio
// https://github.com/boostorg/ratio/blob/818c7febe3cd9e7013e63ecc82d702ea712446d1/include/boost/ratio/detail/overflow_helpers.hpp

// clang-format off
template <typename LHS_T, typename RHS_T, 
          ztl::intmax_t Q_LHS_V = LHS_T::num / LHS_T::den,
          ztl::intmax_t M_LHS_V = LHS_T::num % LHS_T::den, 
          ztl::intmax_t Q_RHS_V = RHS_T::num / RHS_T::den,
          ztl::intmax_t M_RHS_V = RHS_T::num % RHS_T::den>
struct ratio_less_helper {
    static constexpr bool value = Q_LHS_V < Q_RHS_V;
};
// clang-format on

template <typename LHS_T, typename RHS_T, ztl::intmax_t Q_V>
struct ratio_less_helper<LHS_T, RHS_T, Q_V, 0, Q_V, 0> {
    static constexpr bool value = false;
};

template <typename LHS_T, typename RHS_T, ztl::intmax_t Q_V, ztl::intmax_t M_V>
struct ratio_less_helper<LHS_T, RHS_T, Q_V, 0, Q_V, M_V> {
    static constexpr bool value = true;
};

template <typename LHS_T, typename RHS_T, ztl::intmax_t Q_V, ztl::intmax_t M_V>
struct ratio_less_helper<LHS_T, RHS_T, Q_V, M_V, Q_V, 0> {
    static constexpr bool value = false;
};

template <typename LHS_T, typename RHS_T, ztl::intmax_t Q_V, ztl::intmax_t M_LHS_V, ztl::intmax_t M_RHS_V>
struct ratio_less_helper<LHS_T, RHS_T, Q_V, M_LHS_V, Q_V, M_RHS_V> {
    static constexpr bool value = ratio_less_helper<ratio<RHS_T::den, M_RHS_V>, ratio<LHS_T::den, M_LHS_V> >::value;
};

template <typename LHS_V, typename RHS_V, ::ztl::intmax_t SIGN_LHS_V = detail::static_sign_v<LHS_V::num>,
          ::ztl::intmax_t SIGN_RHS_V = detail::static_sign_v<RHS_V::num> >
struct ratio_less : ::ztl::meta::bool_constant<(SIGN_LHS_V < SIGN_RHS_V)> {};

template <typename LHS_V, typename RHS_V>
struct ratio_less<LHS_V, RHS_V, 1, 1> : ::ztl::meta::bool_constant<ratio_less_helper<LHS_V, RHS_V>::value> {};

template <typename LHS_V, typename RHS_V>
struct ratio_less<LHS_V, RHS_V, -1, -1> : ::ztl::meta::bool_constant<ratio_less_helper<RHS_V, LHS_V>::value> {};

}  // namespace detail

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_less : detail::ratio_less<LHS_V, RHS_V> {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_less_v = ratio_less<LHS_V, RHS_V>::value;

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
struct ratio_less_equal : ::ztl::meta::bool_constant<ratio_equal_v<LHS_V, RHS_V> || ratio_less_v<LHS_V, RHS_V> > {};

template <detail::ratio_concept LHS_V, detail::ratio_concept RHS_V>
inline constexpr bool ratio_less_equal_v = ratio_less_equal<LHS_V, RHS_V>::value;
}  // namespace ztl

#endif  // ZTL_NUMERIC_RATIO_RATIO_LESS_HPP
