/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ZTL_TUPLE_TUPLE_BASE_HPP
#define ZTL_TUPLE_TUPLE_BASE_HPP

#include "ztl/types.hpp"
#include "ztl/tuple/detail/tuple_element.hpp"
#include "ztl/tuple/detail/tuple_element_map.hpp"

namespace ztl {

template <typename... ELEMENT_TS>
struct tuple : detail::tuple_map_t<ELEMENT_TS...> {
    using base_type = detail::tuple_map_t<ELEMENT_TS...>;
    using base_type::operator[];

    static constexpr auto size() noexcept -> ztl::size_t { return sizeof...(ELEMENT_TS); }
};

template <>
struct tuple<> : detail::tuple_map_t<> {
    using base_type = detail::tuple_map_t<>;

    static constexpr auto size() noexcept -> ztl::size_t { return 0U; }
};

template <typename... ELEMENT_TS>
tuple(ELEMENT_TS...) -> tuple<ELEMENT_TS...>;
}  // namespace ztl

#endif  // ZTL_TUPLE_TUPLE_BASE_HPP
