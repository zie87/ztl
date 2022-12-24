/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_META_CONCEPT_INTEGRAL_HPP
#define ZTL_META_CONCEPT_INTEGRAL_HPP

#include "ztl/meta/type_trait/is_integral.hpp"

namespace ztl::meta {

template <typename T>
concept integral = is_integral_v<T>;

}  // namespace ztl::meta

#endif  // ZTL_META_CONCEPT_INTEGRAL_HPP
