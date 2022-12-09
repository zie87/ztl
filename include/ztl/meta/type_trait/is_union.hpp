#ifndef ZTL_META_TYPE_TRAIT_IS_UNION_HPP
#define ZTL_META_TYPE_TRAIT_IS_UNION_HPP

#include "ztl/meta/constant/bool_constant.hpp"

namespace ztl::meta {

template <typename T>
struct is_union : bool_constant<__is_union(T)> {};

template <typename T>
inline constexpr bool is_union_v = is_union<T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_UNION_HPP
