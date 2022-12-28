/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_UTILITY_FORWARD_HPP
#define ZTL_UTILITY_FORWARD_HPP

#include "ztl/meta/type_trait/is_reference.hpp"
#include "ztl/meta/type_trait/remove_reference.hpp"

namespace ztl {
template <typename T>
[[nodiscard]] inline constexpr auto forward(meta::remove_reference_t<T>& arg) noexcept -> T&& {
    return static_cast<T&&>(arg);
}

template <typename T>
[[nodiscard]] inline constexpr auto forward(meta::remove_reference_t<T>&& arg) noexcept -> T&& {
    static_assert(!meta::is_lvalue_reference_v<T>, "unable to forward an rvalue as an lvalue");
    return static_cast<T&&>(arg);
}
}  // namespace ztl

#endif  // ZTL_UTILITY_FORWARD_HPP
