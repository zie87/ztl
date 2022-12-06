#ifndef ZTL_META_SEQUENCE_PUSH_FRONT_HPP
#define ZTL_META_SEQUENCE_PUSH_FRONT_HPP

#include "ztl/meta/sequence/base.hpp"
#include "ztl/meta/detail/sequence/push_front.hpp"

namespace ztl::meta {

template <typename ELEMENT_T, typename... ITEM_TS>
struct push_front<sequence<ITEM_TS...>, ELEMENT_T> {
    using type = sequence<ELEMENT_T, ITEM_TS...>;
};

}  // namespace ztl::meta

#endif  // ZTL_META_SEQUENCE_PUSH_FRONT_HPP
