#ifndef ZTL_META_CONCEPT_INTEGRAL_HPP
#define ZTL_META_CONCEPT_INTEGRAL_HPP

#include "ztl/meta/type_trait/is_integral.hpp"

namespace ztl::meta {

template <typename T>
concept integral = is_integral_v<T>;

}  // namespace ztl::meta

#endif  // ZTL_META_CONCEPT_INTEGRAL_HPP
