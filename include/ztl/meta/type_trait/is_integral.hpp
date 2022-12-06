#ifndef ZTL_META_TYPE_TRAIT_IS_INTEGRAL_HPP
#define ZTL_META_TYPE_TRAIT_IS_INTEGRAL_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/meta/detail/type_trait/is_integral.hpp"
#include "ztl/meta/type_trait/remove_cv.hpp"

namespace ztl::meta {

template <typename T>
struct is_integral : bool_constant<detail::is_integral<remove_cv_t<T>>::value> {};

template <typename T>
inline constexpr auto is_integral_v = is_integral<T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_INTEGRAL_HPP
