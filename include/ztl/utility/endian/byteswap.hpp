#ifndef ZTL_UTILITY_ENDIAN_BYTESWAP_HPP
#define ZTL_UTILITY_ENDIAN_BYTESWAP_HPP

#include "ztl/meta/concept/integral.hpp"
#include "ztl/meta/detail/type_trait/always_false.hpp"
#include "ztl/meta/type_trait/has_unique_object_representations.hpp"
#include "ztl/type/stdint.hpp"

namespace ztl {

namespace detail {

// inspired by: https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
template <typename T, ztl::size_t sz>
struct byte_swapper {
    static inline constexpr auto swap(T) noexcept -> T {
        static_assert(meta::detail::always_false<T>, "unexpected integer size for byteswap");
    }
};

template <typename T>
struct byte_swapper<T, 1> {
    static inline constexpr auto swap(T val) noexcept -> T { return val; }
};

template <typename T>
struct byte_swapper<T, 2> {
    static inline constexpr auto swap(T val) noexcept -> T { return ((((val) >> 8) & 0xFF) | (((val)&0xFF) << 8)); }
};

template <typename T>
struct byte_swapper<T, 4> {
    static inline constexpr auto swap(T val) noexcept -> T {
        // clang-format off
        return (((val << 24) & 0xFF00'0000) | 
                ((val << 8)  & 0x00FF'0000) | 
                ((val >> 8)  & 0x0000'FF00) | 
                ((val >> 24) & 0x0000'00FF) );
        // clang-format on
    }
};

template <typename T>
struct byte_swapper<T, 8> {
    static inline constexpr auto swap(T val) noexcept -> T {
        // clang-format off
        return (((val << 56) & 0xFF00'0000'0000'0000) | 
                ((val << 40) & 0x00FF'0000'0000'0000) | 
                ((val << 24) & 0x0000'FF00'0000'0000) | 
                ((val << 8)  & 0x0000'00FF'0000'0000) | 
                ((val >> 8)  & 0x0000'0000'FF00'0000) | 
                ((val >> 24) & 0x0000'0000'00FF'0000) | 
                ((val >> 40) & 0x0000'0000'0000'FF00) | 
                ((val >> 56) & 0x0000'0000'0000'00FF) );
        // clang-format on
    }
};

}  // namespace detail

template <meta::integral T>
requires meta::has_unique_object_representations_v<T>
[[nodiscard]] constexpr auto byteswap(T val) noexcept -> T { return detail::byte_swapper<T, sizeof(T)>::swap(val); }

}  // namespace ztl

#endif  // ZTL_UTILITY_ENDIAN_BYTESWAP_HPP
