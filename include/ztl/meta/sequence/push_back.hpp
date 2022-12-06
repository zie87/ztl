#ifndef ZTL_META_SEQUENCE_PUSH_BACK_HPP
#define ZTL_META_SEQUENCE_PUSH_BACK_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/push_back.hpp"

namespace ztl::meta {

template <typename ELEMENT_T, typename... ITEM_TS>
struct push_back<sequence<ITEM_TS...>, ELEMENT_T> {
    using type = sequence<ITEM_TS..., ELEMENT_T>;
};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_PUSH_BACK_HPP
