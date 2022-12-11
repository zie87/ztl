#ifndef ZTL_NUMERIC_DETAIL_RATIO_STATIC_SIGN_HPP
#define ZTL_NUMERIC_DETAIL_RATIO_STATIC_SIGN_HPP

#include "ztl/meta/constant/integral_constant.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {

consteval auto static_sign_help(::ztl::intmax_t val) noexcept {
    if (val == 0) {
        return 0;
    }
    return (val < 0) ? -1 : 1;
}

template <::ztl::intmax_t V>
struct static_sign : ::ztl::meta::integral_constant<::ztl::intmax_t, static_sign_help(V)> {};

template <::ztl::intmax_t V>
inline constexpr ::ztl::intmax_t static_sign_v = static_sign<V>::value;
}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_RATIO_STATIC_SIGN_HPP
