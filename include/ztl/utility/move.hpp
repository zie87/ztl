/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_UTILITY_MOVE_HPP
#define ZTL_UTILITY_MOVE_HPP

#include "ztl/meta/type_trait/remove_reference.hpp"

namespace ztl {

template <typename T>
[[nodiscard]] inline constexpr auto move(T&& arg) noexcept -> meta::remove_reference_t<T>&& {
    using out_type = meta::remove_reference_t<T>;
    return static_cast<out_type&&>(arg);
}

}  // namespace ztl

#endif  // ZTL_UTILITY_MOVE_HPP
