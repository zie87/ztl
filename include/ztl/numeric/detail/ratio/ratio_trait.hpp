#ifndef ZTL_NUMERIC_DETAIL_RATIO_RATIO_TRAIT_HPP
#define ZTL_NUMERIC_DETAIL_RATIO_RATIO_TRAIT_HPP

#include "ztl/numeric/ratio/ratio.hpp"
#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl::detail {

template <typename>
struct is_ratio : ztl::meta::false_type {};

template <ztl::intmax_t NUM_V, ztl::intmax_t DEN_V>
struct is_ratio<ztl::ratio<NUM_V, DEN_V> > : ztl::meta::true_type {};

template <typename T>
inline constexpr bool is_ratio_v = is_ratio<T>::value;

}  // namespace ztl::detail

#endif  // ZTL_NUMERIC_DETAIL_RATIO_RATIO_TRAIT_HPP
