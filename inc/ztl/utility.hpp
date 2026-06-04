// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UTILITY_HPP
#define ZTL_UTILITY_HPP

#include "ztl/macros.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace ztl
{

// NOTE: Equivalent to C++17 std::size(container)
template<typename CONTAINER_T>
inline constexpr auto size(const CONTAINER_T& container) noexcept(noexcept(container.size()))
    -> decltype(container.size())
{
    return container.size();
}

// NOTE: Equivalent to C++17 std::size(array)
template<typename T, std::size_t N>
inline constexpr auto size(const T (&)[N]) noexcept -> std::size_t
{
    return N;
}

// NOTE: Equivalent to C++20 std::ssize(container)
template<typename CONTAINER_T>
inline constexpr auto ssize(const CONTAINER_T& container) noexcept(noexcept(container.size())) ->
    typename std::common_type<std::ptrdiff_t,
                              typename std::make_signed<decltype(container.size())>::type>::type
{
    typedef
        typename std::common_type<std::ptrdiff_t,
                                  typename std::make_signed<decltype(container.size())>::type>::type
            result_type;

    return static_cast<result_type>(container.size());
}

// NOTE: Equivalent to C++20 std::ssize(array)
template<typename T, std::size_t N>
inline constexpr auto ssize(const T (&)[N]) noexcept -> std::ptrdiff_t
{
    return static_cast<std::ptrdiff_t>(N);
}

} // namespace ztl

#include "ztl/utility/final.hpp"
#include "ztl/utility/nontype.hpp"

#endif // ZTL_UTILITY_HPP
