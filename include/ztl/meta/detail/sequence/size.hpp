#ifndef ZTL_META_DETAIL_SEQUENCE_SIZE_HPP
#define ZTL_META_DETAIL_SEQUENCE_SIZE_HPP

namespace ztl::meta {

template <typename>
struct size;

template <typename LIST_T>
inline constexpr auto size_v = size<LIST_T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_DETAIL_SEQUENCE_SIZE_HPP
