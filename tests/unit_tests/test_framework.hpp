/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_TEST_FRAMEWORK_HPP
#define ZTL_TEST_FRAMEWORK_HPP

#include "UnitTest++/UnitTest++.h"

#define UT_CHECK UNITTEST_CHECK
#define UT_CHECK_TRUE UNITTEST_CHECK
#define UT_CHECK_FALSE(value) UNITTEST_CHECK(!(value))

#define UT_CHECK_EQUAL UNITTEST_CHECK_EQUAL

#endif  // ZTL_TEST_FRAMEWORK_HPP
