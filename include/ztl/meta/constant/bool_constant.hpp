#ifndef ZTL_META_CONSTANT_BOOL_CONSTANT_HPP
#define ZTL_META_CONSTANT_BOOL_CONSTANT_HPP

#include "ztl/meta/detail/constant/type_constant.hpp"

namespace ztl::meta {

template <bool BOOL_V>
using bool_constant = detail::type_constant<bool, BOOL_V>;

using true_type  = bool_constant<true>;
using false_type = bool_constant<false>;

}  // namespace ztl::meta

#endif  // ZTL_META_CONSTANT_BOOL_CONSTANT_HPP
