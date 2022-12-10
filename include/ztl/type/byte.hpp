#ifndef ZTL_TYPE_BYTE_HPP
#define ZTL_TYPE_BYTE_HPP

#include "ztl/meta/concept/integral.hpp"

namespace ztl {
enum class byte : unsigned char {};

[[nodiscard]] constexpr auto operator|(byte lhs, byte rhs) noexcept -> byte {
    return byte(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

[[nodiscard]] constexpr auto operator&(byte lhs, byte rhs) noexcept -> byte {
    return byte(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

[[nodiscard]] constexpr auto operator^(byte lhs, byte rhs) noexcept -> byte {
    return byte(static_cast<unsigned int>(lhs) ^ static_cast<unsigned int>(rhs));
}

[[nodiscard]] constexpr auto operator~(byte b) noexcept -> byte { return byte(~static_cast<unsigned int>(b)); }

constexpr auto operator|=(byte& lhs, byte rhs) noexcept -> byte& { return lhs = (lhs | rhs); }
constexpr auto operator&=(byte& lhs, byte rhs) noexcept -> byte& { return lhs = (lhs & rhs); }
constexpr auto operator^=(byte& lhs, byte rhs) noexcept -> byte& { return lhs = (lhs ^ rhs); }

template <meta::integral T>
[[nodiscard]] constexpr auto operator<<(byte b, T shift) noexcept -> byte {
    return byte(static_cast<unsigned int>(b) << shift);
}

template <meta::integral T>
[[nodiscard]] constexpr auto operator>>(byte b, T shift) noexcept -> byte {
    return byte(static_cast<unsigned int>(b) >> shift);
}

template <meta::integral T>
constexpr auto operator<<=(byte& b, T shift) noexcept -> byte& {
    return b = b << shift;
}

template <meta::integral T>
constexpr auto operator>>=(byte& b, T shift) noexcept -> byte& {
    return b = b >> shift;
}

template <meta::integral T>
[[nodiscard]] constexpr auto to_integer(byte b) noexcept -> T {
    return T(b);
}

}  // namespace ztl

#endif  // ZTL_TYPE_BYTE_HPP
