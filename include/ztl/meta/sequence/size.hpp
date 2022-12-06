#ifndef ZTL_META_SEQUENCE_SIZE_HPP
#define ZTL_META_SEQUENCE_SIZE_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/size.hpp"

#include "ztl/meta/constant/integral_constant.hpp"

namespace ztl::meta {

template <typename... ELEMENT_TS>
struct size<sequence<ELEMENT_TS...>> : integral_constant<unsigned, sizeof...(ELEMENT_TS)> {};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_SIZE_HPP
