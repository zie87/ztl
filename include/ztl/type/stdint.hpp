/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TYPE_STDINT_HPP
#define ZTL_TYPE_STDINT_HPP

#include <stdint.h>

namespace ztl {

using ::int16_t;
using ::int32_t;
using ::int64_t;
using ::int8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;
using ::uint8_t;

using ::int_least16_t;
using ::int_least32_t;
using ::int_least64_t;
using ::int_least8_t;
using ::uint_least16_t;
using ::uint_least32_t;
using ::uint_least64_t;
using ::uint_least8_t;

using ::int_fast16_t;
using ::int_fast32_t;
using ::int_fast64_t;
using ::int_fast8_t;
using ::uint_fast16_t;
using ::uint_fast32_t;
using ::uint_fast64_t;
using ::uint_fast8_t;

using ::intmax_t;
using ::intptr_t;
using ::uintmax_t;
using ::uintptr_t;

}  // namespace ztl

#endif  // ZTL_TYPE_STDINT_HPP
