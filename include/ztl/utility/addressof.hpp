/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_UTILITY_ADDRESSOF_HPP
#define ZTL_UTILITY_ADDRESSOF_HPP

namespace ztl {
namespace detail {
template <typename T>
inline constexpr auto addressof(T& arg) noexcept -> T* {
    return __builtin_addressof(arg);
}
}  // namespace detail

template <typename T>
[[nodiscard]] inline constexpr auto addressof(T& arg) noexcept -> T* {
    return detail::addressof(arg);
}

template <typename T>
inline constexpr auto addressof(const T&& arg) noexcept -> T* = delete;

}  // namespace ztl

#endif  // ZTL_UTILITY_ADDRESSOF_HPP
