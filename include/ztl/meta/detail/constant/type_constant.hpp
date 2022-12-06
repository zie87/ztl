#ifndef ZTL_META_DETAIL_CONSTANT_TYPE_CONSTANT_HPP
#define ZTL_META_DETAIL_CONSTANT_TYPE_CONSTANT_HPP

namespace ztl::meta::detail {

template <typename T, T V>
struct type_constant {
    using type       = type_constant<T, V>;
    using value_type = T;

    static constexpr value_type value = V;

    constexpr operator value_type() const noexcept { return value; }
    constexpr auto operator()() const noexcept -> value_type { return value; }
};

}  // namespace ztl::meta::detail

#endif  // ZTL_META_DETAIL_CONSTANT_TYPE_CONSTANT_HPP
