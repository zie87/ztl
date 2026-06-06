// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UTILITY_NONTYPE_HPP
#define ZTL_UTILITY_NONTYPE_HPP

namespace ztl {

// C++14 replacement for C++17 template<auto V> nontype_t<V>.
// Use as: ZTL_NONTYPE(&free_function), ZTL_NONTYPE(&Class::member).
template <typename T, T Value>
struct nontype_t {
    using value_type = T;
    explicit constexpr nontype_t() noexcept {}
};

template <typename T>
struct is_nontype : std::false_type {};

template <typename T, T Value>
struct is_nontype<nontype_t<T, Value>> : std::true_type {};

} // namespace ztl

#ifndef ZTL_NONTYPE
#define ZTL_NONTYPE(value)                                                                                             \
    ::ztl::nontype_t<decltype(value), (value)> {}
#endif

#endif // ZTL_UTILITY_NONTYPE_HPP
