#ifndef ZTL_META_DETAIL_TYPE_TRAIT_IS_INTEGRAL_HPP
#define ZTL_META_DETAIL_TYPE_TRAIT_IS_INTEGRAL_HPP

namespace ztl::meta::detail {

// clang-format off
template <typename T> struct is_integral { static constexpr bool value = false; };

template <> struct is_integral<bool> { static constexpr bool value = true; };
template <> struct is_integral<char> { static constexpr bool value = true; };
template <> struct is_integral<signed char> { static constexpr bool value = true; };
template <> struct is_integral<unsigned char> { static constexpr bool value = true; };
template <> struct is_integral<wchar_t> { static constexpr bool value = true; };

template <> struct is_integral<char8_t> { static constexpr bool value = true; };
template <> struct is_integral<char16_t> { static constexpr bool value = true; };
template <> struct is_integral<char32_t> { static constexpr bool value = true; };

template <> struct is_integral<short> { static constexpr bool value = true; };
template <> struct is_integral<unsigned short> { static constexpr bool value = true; };

template <> struct is_integral<int> { static constexpr bool value = true; };
template <> struct is_integral<unsigned int> { static constexpr bool value = true; };

template <> struct is_integral<long> { static constexpr bool value = true; };
template <> struct is_integral<unsigned long> { static constexpr bool value = true; };

template <> struct is_integral<long long> { static constexpr bool value = true; };
template <> struct is_integral<unsigned long long> { static constexpr bool value = true; };
// clang-format on

}  // namespace ztl::meta::detail

#endif  // ZTL_META_DETAIL_TYPE_TRAIT_IS_INTEGRAL_HPP
