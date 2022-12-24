/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_UTILITY_ENDIAN_ENDIAN_HPP
#define ZTL_UTILITY_ENDIAN_ENDIAN_HPP

namespace ztl {

// FIXME: move configs into a proper configuration file
enum class endian {
    little = ZTL_CONFIG_LITTLE_ENDIAN,
    big    = ZTL_CONFIG_BIG_ENDIAN,
    native = ZTL_CONFIG_NATIVE_ENDIAN,
};

static_assert((endian::big != endian::little));
static_assert((endian::native == endian::little) || (endian::native == endian::big));

}  // namespace ztl

#endif  // ZTL_UTILITY_ENDIAN_ENDIAN_HPP
