/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_TYPE_STDDEF_HPP
#define ZTL_TYPE_STDDEF_HPP

#include <stddef.h>

namespace ztl {

using ::ptrdiff_t;
using ::size_t;
using nullptr_t = decltype(nullptr);

}  // namespace ztl

#endif  // ZTL_TYPE_STDDEF_HPP
