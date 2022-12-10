#ifndef ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP
#define ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP

namespace ztl::meta::detail {

template <typename T>
inline constexpr bool always_false = false;

}

#endif  // ZTL_META_DETAIL_TYPE_TRAIT_ALWAYS_FALSE_HPP
