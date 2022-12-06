#ifndef ZTL_META_DETAIL_SEQUENCE_PUSH_FRONT_HPP
#define ZTL_META_DETAIL_SEQUENCE_PUSH_FRONT_HPP

namespace ztl::meta {

template <typename, typename>
struct push_front;

template <typename LIST_T, typename ELEMENT_T>
using push_front_t = typename push_front<LIST_T, ELEMENT_T>::type;

}  // namespace ztl::meta

#endif  // ZTL_META_DETAIL_SEQUENCE_PUSH_FRONT_HPP
