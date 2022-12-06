#ifndef ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP
#define ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP

#include "ztl/meta/detail/constant/type_constant.hpp"
#include "ztl/meta/concept/integral.hpp"

namespace ztl::meta {

template <integral T, T V>
struct integral_constant : detail::type_constant<T, V> {};

}  // namespace ztl::meta

#endif  // ZTL_META_CONSTANT_INTEGRAL_CONSTANT_HPP
