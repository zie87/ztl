// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UNITS_BASICUNIT_HPP
#define ZTL_UNITS_BASICUNIT_HPP

#include "ztl/units/ratio.hpp"
#include "ztl/units/unit_trait.hpp"
#include "ztl/numeric.hpp"

#include <numeric>
#include <type_traits>
#include <cstdint>
#include <limits>

namespace ztl { namespace units {

template <typename REP_T, typename UNIT_T>
struct basic_unit;

template <typename T>
struct is_unit : std::false_type {};
template <typename R, typename U>
struct is_unit<basic_unit<R, U>> : std::true_type {};

template <typename T>
struct unit_trait_of;
template <typename R, typename U>
struct unit_trait_of<basic_unit<R, U>> {
    using type = U;
};
template <typename T>
using unit_trait_of_t = typename unit_trait_of<T>::type;

template <typename R, typename U>
struct quantity_of<basic_unit<R, U>, void> {
    using type = quantity_of_t<U>;
};

template <typename R, typename U>
struct ratio_of<basic_unit<R, U>, void> {
    using type = ratio_of_t<U>;
};

template <typename R, typename U>
struct offset_of<basic_unit<R, U>, void> {
    using type = offset_of_t<U>;
};

template <typename R, typename U>
struct offset_value_of<basic_unit<R, U>, void> {
    static constexpr auto value = offset_value_of<U>::value;
};

// --- lossless conversion check -----------------------------------------------
template <typename TARGET_T, typename SOURCE_T, typename = void>
struct is_lossless : std::false_type {};

template <typename TARGET_T, typename SOURCE_T>
struct is_lossless<TARGET_T, SOURCE_T,
                   typename std::enable_if<is_unit<TARGET_T>::value && is_unit<SOURCE_T>::value &&
                                           std::is_same<quantity_of_t<TARGET_T>, quantity_of_t<SOURCE_T>>::value &&
                                           (std::ratio_divide<ratio_of_t<SOURCE_T>, ratio_of_t<TARGET_T>>::num %
                                                std::ratio_divide<ratio_of_t<SOURCE_T>, ratio_of_t<TARGET_T>>::den ==
                                            0)>::type> : std::true_type {};

namespace details {
// --- promote-to-bigger type (portable, no per-type specializations) ----------
// --- overflow detection (handles both signed and unsigned) --------------------
template <std::intmax_t FACTOR, typename T>
constexpr bool would_overflow_positive(T val) {
    return (FACTOR > std::numeric_limits<T>::max()) ||
           (val > static_cast<T>(0) && val > std::numeric_limits<T>::max() / static_cast<T>(FACTOR));
}

template <std::intmax_t FACTOR, typename T>
constexpr bool would_overflow_negative(T val, std::true_type /*is_signed*/) {
    return val < static_cast<T>(0) && val < std::numeric_limits<T>::min() / static_cast<T>(FACTOR);
}

template <std::intmax_t, typename T>
constexpr bool would_overflow_negative(T, std::false_type /*unsigned*/) {
    return false;
}

template <std::intmax_t FACTOR, typename T>
constexpr bool would_overflow(T val) {
    return (FACTOR == 0 || FACTOR == 1)
               ? false
               : would_overflow_positive<FACTOR>(val) || would_overflow_negative<FACTOR>(val, std::is_signed<T>{});
}

//     Picks the next-wider standard integer.  At max width, stays put.
template <std::size_t SIZE, bool SIGNED>
struct bigger_by_size;
template <>
struct bigger_by_size<1, false> {
    using type = std::uint16_t;
};
template <>
struct bigger_by_size<2, false> {
    using type = std::uint32_t;
};
template <>
struct bigger_by_size<4, false> {
    using type = std::uint64_t;
};
// NOTE: at 64-bit width the promotion is a no-op — there is no standard
// type wider than (u)int64_t in C++14.  Overflow protection therefore
// degrades silently for 64-bit reps.  If your toolchain offers __int128,
// consider adding a specialization here.
template <>
struct bigger_by_size<8, false> {
    using type = std::uint64_t;
};
template <>
struct bigger_by_size<1, true> {
    using type = std::int16_t;
};
template <>
struct bigger_by_size<2, true> {
    using type = std::int32_t;
};
template <>
struct bigger_by_size<4, true> {
    using type = std::int64_t;
};
template <>
struct bigger_by_size<8, true> {
    using type = std::int64_t;
};

template <typename T>
using bigger_type_t = typename bigger_by_size<sizeof(T), std::is_signed<T>::value>::type;

template <typename R1, typename R2, typename R3>
struct ratio_common_denominator {
    static constexpr std::intmax_t value = ::ztl::lcm(::ztl::lcm(R1::den, R2::den), R3::den);
};

template <typename SCALE, typename TARGET_OFFSET_SCALE, typename SOURCE_OFFSET_SCALE, typename T>
constexpr T ratio_affine_scale(T val, std::intmax_t target_offset, std::intmax_t source_offset) {
    constexpr std::intmax_t den = ratio_common_denominator<SCALE, TARGET_OFFSET_SCALE, SOURCE_OFFSET_SCALE>::value;

    return static_cast<T>((static_cast<std::intmax_t>(val) * SCALE::num * (den / SCALE::den) +
                           target_offset * TARGET_OFFSET_SCALE::num * (den / TARGET_OFFSET_SCALE::den) -
                           source_offset * SOURCE_OFFSET_SCALE::num * (den / SOURCE_OFFSET_SCALE::den)) /
                          den);
}

} // namespace details

template <typename TARGET_T, typename SOURCE_T>
constexpr typename std::enable_if<is_unit<TARGET_T>::value && is_unit<SOURCE_T>::value &&
                                      std::is_same<quantity_of_t<TARGET_T>, quantity_of_t<SOURCE_T>>::value,
                                  TARGET_T>::type
unit_cast(const SOURCE_T& v) {
    using source_rep = typename SOURCE_T::rep;
    using target_rep = typename TARGET_T::rep;

    using source_unit_trait = unit_trait_of_t<SOURCE_T>;
    using target_unit_trait = unit_trait_of_t<TARGET_T>;

    using source_ratio = ratio_of_t<source_unit_trait>;
    using target_ratio = ratio_of_t<target_unit_trait>;

    using scale_raw = std::ratio_divide<source_ratio, target_ratio>;
    using scale     = details::ratio_scale_down_t<scale_raw, sizeof(target_rep) * 8>;

    using source_offset_ratio = typename offset_of_t<source_unit_trait>::ratio_type;
    using target_offset_ratio = typename offset_of_t<target_unit_trait>::ratio_type;

    using source_offset_scale = std::ratio_divide<source_offset_ratio, target_ratio>;
    using target_offset_scale = std::ratio_divide<target_offset_ratio, target_ratio>;

    using common     = typename std::common_type<source_rep, target_rep>::type;
    const common val = static_cast<common>(v.count());

    // fast path: no risk of overflow
    return (scale::num == 1 || !details::would_overflow<scale::num>(val))
               ? TARGET_T(static_cast<target_rep>(
                     details::ratio_affine_scale<scale, target_offset_scale, source_offset_scale>(
                         val, offset_value_of<target_unit_trait>::value, offset_value_of<source_unit_trait>::value)))
               // slow path: promote to wider type
               : TARGET_T(static_cast<target_rep>(
                     details::ratio_affine_scale<scale, target_offset_scale, source_offset_scale>(
                         static_cast<details::bigger_type_t<common>>(val), offset_value_of<target_unit_trait>::value,
                         offset_value_of<source_unit_trait>::value)));
}

template <typename REP, typename UNIT>
struct basic_unit {
    static_assert(is_unit_trait<UNIT>::value, "UNIT must be a unit_trait");
    static_assert(std::is_integral<REP>::value, "REP must be an integral type");

    using unit_trait = UNIT;

    // --- chrono-aligned typedefs ---
    using rep         = REP;
    using period      = ratio_of_t<UNIT>; // like duration::period
    using offset_type = offset_of_t<UNIT>;

    static constexpr std::intmax_t offset_value = offset_value_of<UNIT>::value;

    // --- constructors ---
    constexpr basic_unit() : m_value() {}
    constexpr explicit basic_unit(REP val) : m_value(val) {}

    // Implicit converting constructor — only for lossless conversions
    // (mirrors duration's implicit narrowing guard)
    template <typename REP2, typename UNIT2>
    constexpr basic_unit(const basic_unit<REP2, UNIT2>& other,
                         typename std::enable_if<is_lossless<basic_unit, basic_unit<REP2, UNIT2>>::value>::type* = 0)
        : m_value(unit_cast<basic_unit>(other).count()) {}

    // --- observers (chrono interface) ---
    constexpr REP count() const { return m_value; }

    static constexpr basic_unit zero() { return basic_unit(REP(0)); }
    static constexpr basic_unit min() { return basic_unit(std::numeric_limits<REP>::min()); }
    static constexpr basic_unit max() { return basic_unit(std::numeric_limits<REP>::max()); }

    // --- compound assignment (same-unit only, like duration) ---
    constexpr basic_unit& operator+=(const basic_unit& rhs) {
        m_value += rhs.m_value;
        return *this;
    }
    constexpr basic_unit& operator-=(const basic_unit& rhs) {
        m_value -= rhs.m_value;
        return *this;
    }
    constexpr basic_unit& operator*=(REP s) {
        m_value *= s;
        return *this;
    }
    constexpr basic_unit& operator/=(REP s) {
        m_value /= s;
        return *this;
    }
    constexpr basic_unit& operator%=(REP s) {
        m_value %= s;
        return *this;
    }
    constexpr basic_unit& operator%=(const basic_unit& rhs) {
        m_value %= rhs.m_value;
        return *this;
    }

    // unary
    constexpr basic_unit operator+() const { return *this; }
    constexpr basic_unit operator-() const { return basic_unit(static_cast<REP>(-m_value)); }

    // pre/post increment (like duration)
    constexpr basic_unit& operator++() {
        ++m_value;
        return *this;
    }
    constexpr basic_unit operator++(int) {
        basic_unit t(*this);
        ++m_value;
        return t;
    }
    constexpr basic_unit& operator--() {
        --m_value;
        return *this;
    }
    constexpr basic_unit operator--(int) {
        basic_unit t(*this);
        --m_value;
        return t;
    }

private:
    REP m_value;
};

// ============================================================================
//  Part 6 — common_type  (mirrors std::common_type for chrono::duration)
// ============================================================================
//  GCD-based: picks the finest ratio representable by both units.
//  For units with offsets, also computes a common offset ratio.

namespace details {

template <typename U1, typename U2>
struct common_unit {
    // GCD-based finest common ratio from both value scalings.
    using ratio1       = ratio_of_t<U1>;
    using ratio2       = ratio_of_t<U2>;
    using common_ratio = std::ratio<gcd(ratio1::num, ratio2::num), lcm(ratio1::den, ratio2::den)>;

    // The common type always uses zero offset (absolute base, e.g. kelvin for
    // temperature).  This avoids the old bug where two affine offsets were
    // summed into a physically meaningless value, and it is correct for every
    // use-site: unit_cast already resolves source/target offsets independently,
    // so comparisons and arithmetic work regardless of the common type's own
    // offset.
    using type = unit_trait<quantity_of_t<U1>, common_ratio, offset_trait<0>>;
};

} // namespace details

template <typename U1, typename U2>
struct is_quantity_compatible
    : ztl::bool_constant<is_unit<U1>::value && is_unit<U2>::value && is_same_quantity<U1, U2>::value> {};

// Additive compatibility: same quantity AND both operands have zero offset.
// Adding two absolute affine values (e.g. celsius + celsius) is physically
// meaningless; only differences of affine units are valid, and those should
// be represented as offset-free quantities (e.g. kelvin differences).
template <typename U1, typename U2>
struct is_additive_compatible
    : ztl::bool_constant<is_quantity_compatible<U1, U2>::value && (offset_value_of<U1>::value == 0) &&
                         (offset_value_of<U2>::value == 0)> {};

template <typename U1, typename U2, typename = void>
struct common_type; // undefined for non-Value types

template <typename U1, typename U2>
struct common_type<U1, U2, typename std::enable_if<is_quantity_compatible<U1, U2>::value>::type> {
    using unit_type = typename details::common_unit<unit_trait_of_t<U1>, unit_trait_of_t<U2>>::type;
    using rep       = typename std::common_type<typename U1::rep, typename U2::rep>::type;
    using type      = basic_unit<rep, unit_type>;
};

template <typename U1, typename U2>
using common_type_t = typename common_type<U1, U2>::type;

// ============================================================================
//  Part 9 — Comparison operators
// ============================================================================
//  All comparisons convert both operands to the common type first,
//  exactly like chrono::duration.

template <typename LHS_T, typename RHS_T>
constexpr auto operator==(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    using type = common_type_t<LHS_T, RHS_T>;
    return unit_cast<type>(lhs).count() == unit_cast<type>(rhs).count();
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator!=(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return !(lhs == rhs);
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator<(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    using type = common_type_t<LHS_T, RHS_T>;
    return unit_cast<type>(lhs).count() < unit_cast<type>(rhs).count();
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator>(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return (rhs < lhs);
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator<=(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return !(lhs > rhs);
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator>=(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return !(lhs < rhs);
}

// ============================================================================
//  Part 10 — Arithmetic operators: same-quantity +/-
// ============================================================================
//  Returns common_type, just like chrono::duration.

template <typename LHS_T, typename RHS_T>
constexpr auto operator+(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_additive_compatible<LHS_T, RHS_T>::value, common_type_t<LHS_T, RHS_T>>::type {
    using type = common_type_t<LHS_T, RHS_T>;
    return type(static_cast<typename type::rep>(unit_cast<type>(lhs).count() + unit_cast<type>(rhs).count()));
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator-(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_additive_compatible<LHS_T, RHS_T>::value, common_type_t<LHS_T, RHS_T>>::type {
    using type = common_type_t<LHS_T, RHS_T>;
    return type(static_cast<typename type::rep>(unit_cast<type>(lhs).count() - unit_cast<type>(rhs).count()));
}

// ============================================================================
//  Part 11 — Arithmetic operators: scalar * /
// ============================================================================

template <typename V, typename S>
constexpr auto operator*(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(static_cast<typename V::rep>(v.count() * s));
}

template <typename S, typename V>
constexpr auto operator*(S s, const V& v) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(static_cast<typename V::rep>(v.count() * s));
}

template <typename V, typename S>
constexpr auto operator/(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(static_cast<typename V::rep>(v.count() / s));
}

template <typename V, typename S>
constexpr auto operator%(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(static_cast<typename V::rep>(v.count() % s));
}

// Same-quantity division: returns the raw rep (a dimensionless scalar),
// matching how dividing two durations of the same period yields Rep.
template <typename LHS_T, typename RHS_T>
constexpr auto operator/(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value && (offset_value_of<LHS_T>::value == 0) &&
                                (offset_value_of<RHS_T>::value == 0),
                            typename LHS_T::rep>::type {
    using result_type = common_type_t<LHS_T, RHS_T>;
    return static_cast<typename LHS_T::rep>(unit_cast<result_type>(lhs).count() / unit_cast<result_type>(rhs).count());
}

// ============================================================================
//  Part 4 — Quantity algebra (multiply / divide)
// ============================================================================

//  Specialise these for every legal quantity product / quotient.
//  Undefined combinations produce a hard compile error — by design.
template <typename Q1, typename Q2>
struct quantity_multiply; // ::type
template <typename Q1, typename Q2>
struct quantity_divide; // ::type

template <typename Q1, typename Q2>
using quantity_multiply_t = typename quantity_multiply<Q1, Q2>::type;
template <typename Q1, typename Q2>
using quantity_divide_t = typename quantity_divide<Q1, Q2>::type;

template <typename Q1, typename Q2, typename = void>
struct has_quantity_multiply : std::false_type {};

template <typename Q1, typename Q2>
struct has_quantity_multiply<Q1, Q2, ::ztl::void_t<typename quantity_multiply<Q1, Q2>::type>> : std::true_type {};

template <typename Q1, typename Q2, typename = void>
struct has_quantity_divide : std::false_type {};

template <typename Q1, typename Q2>
struct has_quantity_divide<Q1, Q2, ::ztl::void_t<typename quantity_divide<Q1, Q2>::type>> : std::true_type {};

template <typename LHS_T, typename RHS_T, typename = void>
struct is_cross_quantity_compatible : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct is_cross_quantity_compatible<LHS_T, RHS_T,
                                    typename std::enable_if<is_unit<LHS_T>::value && is_unit<RHS_T>::value>::type>
    : ztl::bool_constant<!std::is_same<quantity_of_t<LHS_T>, quantity_of_t<RHS_T>>::value &&
                         (offset_value_of<LHS_T>::value == 0) && (offset_value_of<RHS_T>::value == 0)> {};

template <typename LHS_T, typename RHS_T, typename = void>
struct is_cross_quantity_multiply_compatible : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct is_cross_quantity_multiply_compatible<
    LHS_T, RHS_T, typename std::enable_if<is_unit<LHS_T>::value && is_unit<RHS_T>::value>::type>
    : ztl::bool_constant<is_cross_quantity_compatible<LHS_T, RHS_T>::value &&
                         has_quantity_multiply<quantity_of_t<LHS_T>, quantity_of_t<RHS_T>>::value> {};

template <typename LHS_T, typename RHS_T, typename = void>
struct is_cross_quantity_divide_compatible : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct is_cross_quantity_divide_compatible<
    LHS_T, RHS_T, typename std::enable_if<is_unit<LHS_T>::value && is_unit<RHS_T>::value>::type>
    : ztl::bool_constant<is_cross_quantity_compatible<LHS_T, RHS_T>::value &&
                         has_quantity_divide<quantity_of_t<LHS_T>, quantity_of_t<RHS_T>>::value> {};

namespace details {
// --- unit multiply / divide --------------------------------------------------
template <typename U1, typename U2, typename = void>
struct unit_multiply;
template <typename U1, typename U2, typename = void>
struct unit_divide;

template <typename U1, typename U2>
struct unit_multiply<U1, U2, typename std::enable_if<is_unit_trait<U1>::value && is_unit_trait<U2>::value>::type> {
    static_assert(offset_value_of<U1>::value == 0 && offset_value_of<U2>::value == 0,
                  "Cannot multiply units that have a non-zero offset");
    using type = unit_trait<quantity_multiply_t<quantity_of_t<U1>, quantity_of_t<U2>>,
                            std::ratio_multiply<ratio_of_t<U1>, ratio_of_t<U2>>>;
};

template <typename U1, typename U2>
struct unit_divide<U1, U2, typename std::enable_if<is_unit_trait<U1>::value && is_unit_trait<U2>::value>::type> {
    static_assert(offset_value_of<U1>::value == 0 && offset_value_of<U2>::value == 0,
                  "Cannot divide units that have a non-zero offset");
    using type = unit_trait<quantity_divide_t<quantity_of_t<U1>, quantity_of_t<U2>>,
                            std::ratio_divide<ratio_of_t<U1>, ratio_of_t<U2>>>;
};

template <typename U1, typename U2>
using unit_multiply_t = typename unit_multiply<U1, U2>::type;
template <typename U1, typename U2>
using unit_divide_t = typename unit_divide<U1, U2>::type;
} // namespace details

// ============================================================================
//  Part 12 — Arithmetic operators: cross-quantity * /
// ============================================================================
//  Value<voltage> * Value<current>  →  Value<power>  (etc.)
//  Only permitted when both operands have zero offset.

template <typename LHS_T, typename RHS_T,
          typename std::enable_if<is_cross_quantity_multiply_compatible<LHS_T, RHS_T>::value, int>::type = 0>
constexpr auto operator*(const LHS_T& lhs, const RHS_T& rhs) noexcept
    -> basic_unit<typename LHS_T::rep, details::unit_multiply_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>> {
    using trait_type  = details::unit_multiply_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>;
    using result_type = basic_unit<typename LHS_T::rep, trait_type>;
    // Promote to a wider type before multiplying to reduce overflow risk
    using wide_type = details::bigger_type_t<typename LHS_T::rep>;
    return result_type(
        static_cast<typename LHS_T::rep>(static_cast<wide_type>(lhs.count()) * static_cast<wide_type>(rhs.count())));
}

template <typename LHS_T, typename RHS_T,
          typename std::enable_if<is_cross_quantity_divide_compatible<LHS_T, RHS_T>::value, int>::type = 0>
constexpr auto operator/(const LHS_T& lhs, const RHS_T& rhs)
    -> basic_unit<typename LHS_T::rep, details::unit_divide_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>> {
    using trait_type  = details::unit_divide_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>;
    using result_type = basic_unit<typename LHS_T::rep, trait_type>;

    // Compute scaling to compensate for ratio differences
    using lhs_scale  = std::ratio_divide<ratio_of_t<unit_trait_of_t<LHS_T>>, ratio_of_t<trait_type>>;
    using full_scale = std::ratio_divide<lhs_scale, ratio_of_t<unit_trait_of_t<RHS_T>>>;

    // Promote to a wider type before the ratio-scaled division
    using wide_type = details::bigger_type_t<typename LHS_T::rep>;
    return result_type(static_cast<typename LHS_T::rep>(
        details::ratio_scale<full_scale>(static_cast<wide_type>(lhs.count())) / static_cast<wide_type>(rhs.count())));
}

}} // namespace ztl::units

#endif // ZTL_UNITS_BASICUNIT_HPP
