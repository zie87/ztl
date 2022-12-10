#ifndef ZTL_META_TYPE_TRAIT_HAS_UNIQUE_OBJECT_REPRESENTATION_HPP
#define ZTL_META_TYPE_TRAIT_HAS_UNIQUE_OBJECT_REPRESENTATION_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/meta/type_trait/remove_all_extents.hpp"
#include "ztl/meta/type_trait/remove_cv.hpp"

namespace ztl::meta {

template <typename T>
struct has_unique_object_representations
    : bool_constant<__has_unique_object_representations(remove_cv_t<remove_all_extents_t<T>>)> {};

template <typename T>
inline constexpr bool has_unique_object_representations_v = has_unique_object_representations<T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_HAS_UNIQUE_OBJECT_REPRESENTATION_HPP
