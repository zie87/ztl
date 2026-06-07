// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UNITS_UNITTRAIT_HPP
#define ZTL_UNITS_UNITTRAIT_HPP

#include "ztl/units/ratio.hpp"
#include "ztl/type_traits.hpp"

#include <ratio>
#include <type_traits>

namespace ztl { namespace units {

// --- offset<Value, Ratio> ----------------------------------------------------
//     Represents an affine shift:  physical_zero = Value * Ratio  above base zero.
//     Example: Celsius → offset<-273150, std::milli>
template <std::intmax_t OFFSET_V = 0, typename RATIO_T = std::ratio<1, 1>>
struct offset_trait {
    static_assert(is_ratio<RATIO_T>::value, "RATIO_T must be a std::ratio");
    static constexpr std::intmax_t value = OFFSET_V;
    using ratio_type                     = RATIO_T;
};

namespace details {
template <typename T, typename = void>
struct has_offset_trait_members : std::false_type {};

template <typename T>
struct has_offset_trait_members<T, ::ztl::void_t<decltype(T::value), typename T::ratio_type>> : std::true_type {};

template <typename T, bool HAS_MEMBERS_V>
struct is_offset_trait_impl : std::false_type {};

template <typename T>
struct is_offset_trait_impl<T, true>
    : std::integral_constant<bool, std::is_convertible<decltype(T::value), std::intmax_t>::value &&
                                       is_ratio<typename T::ratio_type>::value> {};
} // namespace details

template <typename T>
struct is_offset_trait : details::is_offset_trait_impl<T, details::has_offset_trait_members<T>::value> {};

// --- unit<Quantity, Scaling, Offset> -----------------------------------------
//     A physical unit: quantity tag + SI ratio + optional affine offset.
//     Example: millivolt = unit<voltage, std::milli>
template <typename QUANTITY, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
struct unit_trait {
    static_assert(is_ratio<SCALING>::value, "SCALING must be a std::ratio");
    static_assert(is_offset_trait<OFFSET>::value, "OFFSET must be an offset<>");
    using quantity_type                         = QUANTITY;
    using ratio_type                            = SCALING; // period (chrono terminology)
    using offset_type                           = OFFSET;
    static constexpr std::intmax_t offset_value = OFFSET::value;
};

namespace details {

template <typename T, typename = void>
struct has_unit_trait_members : std::false_type {};

template <typename T>
struct has_unit_trait_members<T,
                              ::ztl::void_t<typename T::ratio_type, typename T::offset_type, typename T::quantity_type>>
    : std::true_type {};

template <typename T, bool HAS_MEMBERS_V>
struct is_unit_trait_impl : std::false_type {};

template <typename T>
struct is_unit_trait_impl<T, true> : std::integral_constant<bool, is_ratio<typename T::ratio_type>::value &&
                                                                      is_offset_trait<typename T::offset_type>::value> {
};

} // namespace details

template <typename T>
struct is_unit_trait : details::is_unit_trait_impl<T, details::has_unit_trait_members<T>::value> {};

template <typename T, typename = void>
struct quantity_of; // ::type

template <typename Q, typename S, typename O>
struct quantity_of<unit_trait<Q, S, O>, typename std::enable_if<is_unit_trait<unit_trait<Q, S, O>>::value>::type> {
    using type = Q;
};

template <typename T>
struct quantity_of<T, typename std::enable_if<is_unit_trait<T>::value>::type> {
    using type = typename T::quantity_type;
};

template <typename T>
using quantity_of_t = typename quantity_of<T>::type;

template <typename U1, typename U2, typename = void>
struct is_same_quantity : std::false_type {};

template <typename U1, typename U2>
struct is_same_quantity<U1, U2,
                        typename std::enable_if<std::is_same<quantity_of_t<U1>, quantity_of_t<U2>>::value>::type>
    : std::true_type {};

template <typename T, typename = void>
struct ratio_of; // ::type

template <typename Q, typename S, typename O>
struct ratio_of<unit_trait<Q, S, O>, typename std::enable_if<is_unit_trait<unit_trait<Q, S, O>>::value>::type> {
    using type = S;
};

template <typename T>
struct ratio_of<T, typename std::enable_if<is_unit_trait<T>::value>::type> {
    using type = typename T::ratio_type;
};

template <typename T>
using ratio_of_t = typename ratio_of<T>::type;

template <typename T, typename = void>
struct offset_of; // ::type

template <typename Q, typename S, typename O>
struct offset_of<unit_trait<Q, S, O>, typename std::enable_if<is_unit_trait<unit_trait<Q, S, O>>::value>::type> {
    using type = O;
};

template <typename T>
struct offset_of<T, typename std::enable_if<is_unit_trait<T>::value>::type> {
    using type = typename T::offset_type;
};

template <typename T>
using offset_of_t = typename offset_of<T>::type;

template <typename T, typename = void>
struct offset_value_of; // ::value

template <typename T>
struct offset_value_of<T, typename std::enable_if<is_unit_trait<T>::value>::type> {
    using type                  = typename offset_of<T>::type;
    static constexpr auto value = type::value;
};

}} // namespace ztl::units

#endif // ZTL_UNITS_UNITTRAIT_HPP
