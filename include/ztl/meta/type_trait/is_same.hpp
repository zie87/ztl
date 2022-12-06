#ifndef ZTL_META_TYPE_TRAIT_IS_SAME_HPP
#define ZTL_META_TYPE_TRAIT_IS_SAME_HPP

#include "ztl/meta/constant/bool_constant.hpp"

namespace ztl::meta {

template <typename LHS_T, typename RHS_T>
struct is_same : ::ztl::meta::false_type {};

template <typename T>
struct is_same<T, T> : ::ztl::meta::true_type {};

template <typename LHS_T, typename RHS_T>
inline constexpr bool is_same_v = is_same<LHS_T, RHS_T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_SAME_HPP
