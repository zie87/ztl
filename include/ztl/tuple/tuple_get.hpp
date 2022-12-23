/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_TUPLE_GET_HPP
#define ZTL_TUPLE_TUPLE_GET_HPP

#include "ztl/tuple/detail/tuple_concept.hpp"
#include "ztl/tuple/tuple_base.hpp"
#include "ztl/tuple/tuple_element.hpp"

namespace ztl {

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
[[nodiscard]] constexpr auto get(TUPLE_T& val) noexcept -> ztl::tuple_element_t<IDX_V, TUPLE_T>& {
    using idx = detail::index_type<IDX_V>;
    return val[idx{}];
}

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
[[nodiscard]] constexpr auto get(const TUPLE_T& val) noexcept -> const ztl::tuple_element_t<IDX_V, TUPLE_T>& {
    using idx = detail::index_type<IDX_V>;
    return val[idx{}];
}

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
[[nodiscard]] constexpr auto get(TUPLE_T&& val) noexcept -> ztl::tuple_element_t<IDX_V, TUPLE_T>&& {
    using idx      = detail::index_type<IDX_V>;
    using elm_type = ztl::tuple_element_t<IDX_V, TUPLE_T>;
    return static_cast<elm_type&&>(val[idx{}]);
}

template <ztl::size_t IDX_V, detail::tuple_concept TUPLE_T>
[[nodiscard]] constexpr auto get(const TUPLE_T&& val) noexcept -> const ztl::tuple_element_t<IDX_V, TUPLE_T>&& {
    using idx      = detail::index_type<IDX_V>;
    using elm_type = ztl::tuple_element_t<IDX_V, TUPLE_T>;
    return static_cast<const elm_type&&>(val[idx{}]);
}

}  // namespace ztl

#endif  // ZTL_TUPLE_TUPLE_GET_HPP
