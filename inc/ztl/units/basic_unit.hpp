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
#include "ztl/assert.hpp"
#include "ztl/numeric.hpp"
#include "ztl/details/uint128.hpp"

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

namespace details {
// --- conversion parameters: target = source * scale + offset ------------------
//     Both are exact ratios expressed in target units.
template <typename TARGET_T, typename SOURCE_T>
struct conversion {
    using source_unit_trait = unit_trait_of_t<SOURCE_T>;
    using target_unit_trait = unit_trait_of_t<TARGET_T>;
    using target_ratio      = ratio_of_t<target_unit_trait>;

    using scale = std::ratio_divide<ratio_of_t<source_unit_trait>, target_ratio>;

    using source_offset_scale = std::ratio_divide<typename offset_of_t<source_unit_trait>::ratio_type, target_ratio>;
    using target_offset_scale = std::ratio_divide<typename offset_of_t<target_unit_trait>::ratio_type, target_ratio>;

    // target_offset * target_offset_scale - source_offset * source_offset_scale
    using offset = std::ratio_subtract<
        std::ratio_multiply<std::ratio<offset_value_of<target_unit_trait>::value>, target_offset_scale>,
        std::ratio_multiply<std::ratio<offset_value_of<source_unit_trait>::value>, source_offset_scale>>;
};

// Every value of FROM is representable in TO (the rules of a non-narrowing
// brace-initialisation between integer types).
template <typename TO, typename FROM>
struct is_non_narrowing_rep
    : std::integral_constant<
          bool, (std::is_signed<TO>::value == std::is_signed<FROM>::value && sizeof(TO) >= sizeof(FROM)) ||
                    (std::is_signed<TO>::value && !std::is_signed<FROM>::value && sizeof(TO) > sizeof(FROM))> {};
} // namespace details

// --- lossless conversion check -----------------------------------------------
//     A conversion is lossless (and therefore implicit) when the scale and the
//     offset are whole numbers in the target unit and the rep does not narrow.
//     Like std::chrono::duration, overflow from a large scale is not considered.
template <typename TARGET_T, typename SOURCE_T, typename = void>
struct is_lossless : std::false_type {};

template <typename TARGET_T, typename SOURCE_T>
struct is_lossless<TARGET_T, SOURCE_T,
                   typename std::enable_if<is_unit<TARGET_T>::value && is_unit<SOURCE_T>::value &&
                                           std::is_same<quantity_of_t<TARGET_T>, quantity_of_t<SOURCE_T>>::value>::type>
    : std::integral_constant<bool,
                             details::conversion<TARGET_T, SOURCE_T>::scale::den == 1 &&
                                 details::conversion<TARGET_T, SOURCE_T>::offset::den == 1 &&
                                 details::is_non_narrowing_rep<typename TARGET_T::rep, typename SOURCE_T::rep>::value> {
};

namespace details {
// --- exact affine scaling: result = (val * FACTOR + OFFSET) / DEN ------------
//     FACTOR, OFFSET and DEN are compile-time constants derived from the units.
//     When val * FACTOR + OFFSET fits into intmax_t the plain arithmetic is used;
//     otherwise the product is formed in 128 bits so that no intermediate value
//     overflows. MODE_V selects how the exact quotient is rounded.
enum class rounding {
    toward_zero,     // unit_cast  (std::chrono::duration_cast)
    down,            // unit_floor (std::chrono::floor)
    up,              // unit_ceil  (std::chrono::ceil)
    to_nearest_even, // unit_round (std::chrono::round, ties to even)
};

// Adjusts the truncated magnitude q of an exact quotient with remainder r (0 <= r < den).
template <rounding MODE_V>
constexpr std::uint64_t round_quotient(bool negative, std::uint64_t q, std::uint64_t r, std::uint64_t den) noexcept {
    return r == 0u                    ? q
           : MODE_V == rounding::down ? (negative ? q + 1u : q)
           : MODE_V == rounding::up   ? (negative ? q : q + 1u)
           : MODE_V == rounding::to_nearest_even
               // compare r with den - r instead of 2r with den, which could overflow
               ? ((r > den - r || (r == den - r && (q & 1u) != 0u)) ? q + 1u : q)
               : q;
}

constexpr bool intmax_mul_fits(std::intmax_t a, std::intmax_t b) noexcept {
    return a == 0 || b == 0 ||
           ::ztl::details::abs_val(a) <= std::numeric_limits<std::intmax_t>::max() / ::ztl::details::abs_val(b);
}

template <typename SCALE, typename OFFSET>
struct affine_constants {
    static_assert(sizeof(std::intmax_t) == sizeof(std::uint64_t), "unit_cast assumes a 64-bit intmax_t");

    static constexpr std::intmax_t den_gcd = ::ztl::gcd(SCALE::den, OFFSET::den);
    static_assert(intmax_mul_fits(SCALE::den / den_gcd, OFFSET::den),
                  "unit_cast: the common denominator of scale and offset does not fit into intmax_t");
    static constexpr std::intmax_t den = SCALE::den / den_gcd * OFFSET::den;

    static_assert(intmax_mul_fits(SCALE::num, den / SCALE::den),
                  "unit_cast: the scale factor does not fit into intmax_t");
    static_assert(intmax_mul_fits(OFFSET::num, den / OFFSET::den), "unit_cast: the offset does not fit into intmax_t");
    static constexpr std::intmax_t factor = SCALE::num * (den / SCALE::den);
    static constexpr std::intmax_t offset = OFFSET::num * (den / OFFSET::den);
};

template <typename T>
constexpr std::uint64_t magnitude(T val, std::true_type /*is_signed*/) noexcept {
    return val < 0 ? std::uint64_t{0} - static_cast<std::uint64_t>(val) : static_cast<std::uint64_t>(val);
}

template <typename T>
constexpr std::uint64_t magnitude(T val, std::false_type /*is_signed*/) noexcept {
    return static_cast<std::uint64_t>(val);
}

template <typename T>
constexpr std::uint64_t magnitude(T val) noexcept {
    return magnitude(val, std::is_signed<T>{});
}

// Converts sign + magnitude to REP. Exact whenever the value is representable;
// out-of-range values wrap like a static_cast from intmax_t would.
template <typename REP>
constexpr REP from_sign_magnitude(bool negative, std::uint64_t mag) noexcept {
    return !negative ? static_cast<REP>(mag)
           : mag <= static_cast<std::uint64_t>(std::numeric_limits<std::intmax_t>::max())
               ? static_cast<REP>(-static_cast<std::intmax_t>(mag))
               : static_cast<REP>(static_cast<std::intmax_t>(std::uint64_t{0} - mag));
}

// --- wrap-around integer arithmetic on reps ----------------------------------
//     All unit arithmetic goes through these helpers so that no operation has
//     undefined behaviour. Results that do not fit REP wrap like a static_cast;
//     division and remainder work on sign + magnitude, so mixing signed reps with
//     unsigned scalars keeps the mathematical sign (-9 / 3u == -3).
template <typename REP>
constexpr REP add_reps(REP lhs, REP rhs) noexcept {
    using unsigned_rep = typename std::make_unsigned<REP>::type;
    return static_cast<REP>(static_cast<unsigned_rep>(static_cast<unsigned_rep>(lhs) + static_cast<unsigned_rep>(rhs)));
}

template <typename REP>
constexpr REP subtract_reps(REP lhs, REP rhs) noexcept {
    using unsigned_rep = typename std::make_unsigned<REP>::type;
    return static_cast<REP>(static_cast<unsigned_rep>(static_cast<unsigned_rep>(lhs) - static_cast<unsigned_rep>(rhs)));
}

template <typename REP, typename L, typename R>
constexpr REP multiply_reps(L lhs, R rhs) noexcept {
    const bool negative = (lhs < L{0}) != (rhs < R{0});
    // two magnitudes of at most 32 bits each always fit into 64 bits
    return (sizeof(L) <= 4 && sizeof(R) <= 4)
               ? from_sign_magnitude<REP>(negative, magnitude(lhs) * magnitude(rhs))
               : from_sign_magnitude<REP>(negative, ::ztl::details::uint128_mul(magnitude(lhs), magnitude(rhs)).lo);
}

// Truncates toward zero; the caller checks rhs != 0.
template <typename REP, typename L, typename R>
constexpr REP divide_reps(L lhs, R rhs) noexcept {
    return from_sign_magnitude<REP>((lhs < L{0}) != (rhs < R{0}), magnitude(lhs) / magnitude(rhs));
}

// Sign follows the dividend, like the built-in %; the caller checks rhs != 0.
template <typename REP, typename L, typename R>
constexpr REP remainder_reps(L lhs, R rhs) noexcept {
    return from_sign_magnitude<REP>(lhs < L{0}, magnitude(lhs) % magnitude(rhs));
}

// val * FACTOR + OFFSET as sign + magnitude. |val * FACTOR| < 2^127 and
// |OFFSET| < 2^63, so the magnitude always fits into 128 bits.
struct signed_uint128 {
    bool                    negative;
    ::ztl::details::uint128 magnitude;
};

template <std::intmax_t FACTOR_V, std::intmax_t OFFSET_V, typename T>
constexpr signed_uint128 affine_numerator(T val) noexcept {
    const std::uint64_t offset_mag = magnitude(OFFSET_V);
    const bool          offset_neg = OFFSET_V < 0;

    const ::ztl::details::uint128 product     = ::ztl::details::uint128_mul(magnitude(val), magnitude(FACTOR_V));
    const bool                    product_neg = (val < T{0}) != (FACTOR_V < 0);

    if (product.hi == 0u && product.lo == 0u) {
        return signed_uint128{offset_neg, ::ztl::details::uint128{0u, offset_mag}};
    }
    if (product_neg == offset_neg) {
        return signed_uint128{product_neg, ::ztl::details::uint128_add(product, offset_mag)};
    }
    if (::ztl::details::uint128_less(product, offset_mag)) {
        return signed_uint128{offset_neg, ::ztl::details::uint128{0u, offset_mag - product.lo}};
    }
    return signed_uint128{product_neg, ::ztl::details::uint128_sub(product, offset_mag)};
}

template <rounding MODE_V, std::intmax_t FACTOR_V, std::intmax_t OFFSET_V, std::intmax_t DEN_V, typename REP,
          typename T>
constexpr REP affine_scale_wide(T val) noexcept {
    const signed_uint128                        sum = affine_numerator<FACTOR_V, OFFSET_V>(val);
    const ::ztl::details::uint128_divmod_result division =
        ::ztl::details::uint128_divmod(sum.magnitude, static_cast<std::uint64_t>(DEN_V));
    return from_sign_magnitude<REP>(sum.negative,
                                    round_quotient<MODE_V>(sum.negative, division.quotient.lo, division.remainder,
                                                           static_cast<std::uint64_t>(DEN_V)));
}

// true when val * FACTOR + OFFSET cannot overflow intmax_t
template <std::intmax_t FACTOR_V, std::intmax_t OFFSET_V, typename T>
constexpr bool fits_intmax(T val) noexcept {
    return magnitude(val) <=
           (static_cast<std::uint64_t>(std::numeric_limits<std::intmax_t>::max()) - magnitude(OFFSET_V)) /
               magnitude(FACTOR_V);
}

template <rounding MODE_V, std::intmax_t FACTOR_V, std::intmax_t OFFSET_V, std::intmax_t DEN_V, typename REP,
          typename T>
constexpr REP affine_scale_narrow(std::intmax_t numerator) noexcept {
    return MODE_V == rounding::toward_zero
               ? static_cast<REP>(numerator / DEN_V)
               : from_sign_magnitude<REP>(
                     numerator < 0,
                     round_quotient<MODE_V>(numerator < 0, magnitude(numerator) / static_cast<std::uint64_t>(DEN_V),
                                            magnitude(numerator) % static_cast<std::uint64_t>(DEN_V),
                                            static_cast<std::uint64_t>(DEN_V)));
}

template <rounding MODE_V, std::intmax_t FACTOR_V, std::intmax_t OFFSET_V, std::intmax_t DEN_V, typename REP,
          typename T>
constexpr REP affine_scale(T val) noexcept {
    return fits_intmax<FACTOR_V, OFFSET_V>(val) ? affine_scale_narrow<MODE_V, FACTOR_V, OFFSET_V, DEN_V, REP, T>(
                                                      static_cast<std::intmax_t>(val) * FACTOR_V + OFFSET_V)
                                                : affine_scale_wide<MODE_V, FACTOR_V, OFFSET_V, DEN_V, REP>(val);
}

} // namespace details

namespace details {
template <typename TARGET_T, typename SOURCE_T>
using enable_unit_conversion_t =
    typename std::enable_if<is_unit<TARGET_T>::value && is_unit<SOURCE_T>::value &&
                                std::is_same<quantity_of_t<TARGET_T>, quantity_of_t<SOURCE_T>>::value,
                            TARGET_T>::type;

// --- exact comparison ---------------------------------------------------------
constexpr int compare_magnitude(::ztl::details::uint128 a, ::ztl::details::uint128 b) noexcept {
    return a.hi != b.hi ? (a.hi < b.hi ? -1 : 1) : a.lo != b.lo ? (a.lo < b.lo ? -1 : 1) : 0;
}

constexpr bool is_negative(const signed_uint128& v) noexcept {
    return v.negative && (v.magnitude.hi != 0u || v.magnitude.lo != 0u);
}

constexpr int compare_signed(const signed_uint128& a, const signed_uint128& b) noexcept {
    return is_negative(a) != is_negative(b) ? (is_negative(a) ? -1 : 1)
           : is_negative(a)                 ? compare_magnitude(b.magnitude, a.magnitude)
                                            : compare_magnitude(a.magnitude, b.magnitude);
}

template <typename T>
constexpr int three_way(T a, T b) noexcept {
    return a < b ? -1 : (b < a ? 1 : 0);
}

// Returns <0, 0 or >0 as lhs is less than, equal to or greater than rhs, using
// the exact values (no truncation, overflow or signed/unsigned conversion).
// rhs in lhs units is (rhs * factor + offset) / den, so lhs * den is compared
// with rhs * factor + offset.
template <typename LHS_T, typename RHS_T>
constexpr int compare_units(const LHS_T& lhs, const RHS_T& rhs) noexcept {
    using params    = conversion<LHS_T, RHS_T>;
    using constants = affine_constants<typename params::scale, typename params::offset>;

    return std::is_same<LHS_T, RHS_T>::value ? three_way(lhs.count(), static_cast<typename LHS_T::rep>(rhs.count()))
           : (fits_intmax<constants::den, 0>(lhs.count()) &&
              fits_intmax<constants::factor, constants::offset>(rhs.count()))
               ? three_way(static_cast<std::intmax_t>(lhs.count()) * constants::den,
                           static_cast<std::intmax_t>(rhs.count()) * constants::factor + constants::offset)
               : compare_signed(affine_numerator<constants::den, 0>(lhs.count()),
                                affine_numerator<constants::factor, constants::offset>(rhs.count()));
}

template <rounding MODE_V, typename TARGET_T, typename SOURCE_T>
constexpr TARGET_T convert_unit(const SOURCE_T& v) {
    using target_rep = typename TARGET_T::rep;

    // NOTE: the exact ratios must be used here. The arithmetic does not depend on
    // the width of target_rep, so that width says nothing about how wide a ratio may be.
    using params    = conversion<TARGET_T, SOURCE_T>;
    using constants = affine_constants<typename params::scale, typename params::offset>;
    return TARGET_T(affine_scale<MODE_V, constants::factor, constants::offset, constants::den, target_rep>(v.count()));
}
} // namespace details

// Converts to TARGET_T, truncating toward zero (like std::chrono::duration_cast).
template <typename TARGET_T, typename SOURCE_T>
constexpr details::enable_unit_conversion_t<TARGET_T, SOURCE_T> unit_cast(const SOURCE_T& v) {
    return details::convert_unit<details::rounding::toward_zero, TARGET_T>(v);
}

// Converts to TARGET_T, rounding toward negative infinity (like std::chrono::floor).
template <typename TARGET_T, typename SOURCE_T>
constexpr details::enable_unit_conversion_t<TARGET_T, SOURCE_T> unit_floor(const SOURCE_T& v) {
    return details::convert_unit<details::rounding::down, TARGET_T>(v);
}

// Converts to TARGET_T, rounding toward positive infinity (like std::chrono::ceil).
template <typename TARGET_T, typename SOURCE_T>
constexpr details::enable_unit_conversion_t<TARGET_T, SOURCE_T> unit_ceil(const SOURCE_T& v) {
    return details::convert_unit<details::rounding::up, TARGET_T>(v);
}

// Converts to TARGET_T, rounding to nearest with ties to even (like std::chrono::round).
template <typename TARGET_T, typename SOURCE_T>
constexpr details::enable_unit_conversion_t<TARGET_T, SOURCE_T> unit_round(const SOURCE_T& v) {
    return details::convert_unit<details::rounding::to_nearest_even, TARGET_T>(v);
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
        m_value = details::add_reps(m_value, rhs.m_value);
        return *this;
    }
    constexpr basic_unit& operator-=(const basic_unit& rhs) {
        m_value = details::subtract_reps(m_value, rhs.m_value);
        return *this;
    }
    constexpr basic_unit& operator*=(REP s) {
        m_value = details::multiply_reps<REP>(m_value, s);
        return *this;
    }
    constexpr basic_unit& operator/=(REP s) {
        ZTL_EXPECTS(s != 0);
        m_value = details::divide_reps<REP>(m_value, s);
        return *this;
    }
    constexpr basic_unit& operator%=(REP s) {
        ZTL_EXPECTS(s != 0);
        m_value = details::remainder_reps<REP>(m_value, s);
        return *this;
    }
    constexpr basic_unit& operator%=(const basic_unit& rhs) {
        ZTL_EXPECTS(rhs.m_value != 0);
        m_value = details::remainder_reps<REP>(m_value, rhs.m_value);
        return *this;
    }

    // unary
    constexpr basic_unit operator+() const { return *this; }
    constexpr basic_unit operator-() const { return basic_unit(details::subtract_reps(REP(0), m_value)); }

    // pre/post increment (like duration)
    constexpr basic_unit& operator++() {
        m_value = details::add_reps(m_value, REP(1));
        return *this;
    }
    constexpr basic_unit operator++(int) {
        basic_unit t(*this);
        ++*this;
        return t;
    }
    constexpr basic_unit& operator--() {
        m_value = details::subtract_reps(m_value, REP(1));
        return *this;
    }
    constexpr basic_unit operator--(int) {
        basic_unit t(*this);
        --*this;
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
    // temperature). It is only used for +, - and same-quantity /, which are
    // restricted to zero-offset units; comparisons do not go through it (see
    // details::compare_units), so truncating an offset here cannot affect them.
    using type = unit_trait<quantity_of_t<U1>, common_ratio, offset_trait<0>>;
};

} // namespace details

// NOTE: conjunction short-circuits the instantiation, so these traits stay false (instead of
// a hard error) for non-unit types. The unconstrained operator templates below are found by
// ADL for any type that merely mentions a unit in its template arguments (e.g. mocks).
template <typename U1, typename U2>
struct is_quantity_compatible : ztl::conjunction<is_unit<U1>, is_unit<U2>, is_same_quantity<U1, U2>> {};

template <typename U>
struct has_zero_offset : ztl::bool_constant<offset_value_of<U>::value == 0> {};

// Additive compatibility: same quantity AND both operands have zero offset.
// Adding two absolute affine values (e.g. celsius + celsius) is physically
// meaningless; only differences of affine units are valid, and those should
// be represented as offset-free quantities (e.g. kelvin differences).
template <typename U1, typename U2>
struct is_additive_compatible
    : ztl::conjunction<is_quantity_compatible<U1, U2>, has_zero_offset<U1>, has_zero_offset<U2>> {};

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
//  Comparisons use the exact values (see details::compare_units), so they never
//  truncate offsets, overflow the common rep or mix signed/unsigned reps.

template <typename LHS_T, typename RHS_T>
constexpr auto operator==(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return details::compare_units(lhs, rhs) == 0;
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator!=(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return !(lhs == rhs);
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator<(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value, bool>::type {
    return details::compare_units(lhs, rhs) < 0;
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
    return type(details::add_reps(unit_cast<type>(lhs).count(), unit_cast<type>(rhs).count()));
}

template <typename LHS_T, typename RHS_T>
constexpr auto operator-(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_additive_compatible<LHS_T, RHS_T>::value, common_type_t<LHS_T, RHS_T>>::type {
    using type = common_type_t<LHS_T, RHS_T>;
    return type(details::subtract_reps(unit_cast<type>(lhs).count(), unit_cast<type>(rhs).count()));
}

// ============================================================================
//  Part 11 — Arithmetic operators: scalar * /
// ============================================================================

template <typename V, typename S>
constexpr auto operator*(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(details::multiply_reps<typename V::rep>(v.count(), s));
}

template <typename S, typename V>
constexpr auto operator*(S s, const V& v) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    return V(details::multiply_reps<typename V::rep>(v.count(), s));
}

template <typename V, typename S>
constexpr auto operator/(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    ZTL_EXPECTS(s != 0);
    return V(details::divide_reps<typename V::rep>(v.count(), s));
}

template <typename V, typename S>
constexpr auto operator%(const V& v, S s) noexcept ->
    typename std::enable_if<is_unit<V>::value && std::is_integral<S>::value, V>::type {
    ZTL_EXPECTS(s != 0);
    return V(details::remainder_reps<typename V::rep>(v.count(), s));
}

// Same-quantity division: returns the raw rep (a dimensionless scalar),
// matching how dividing two durations of the same period yields Rep.
template <typename LHS_T, typename RHS_T>
constexpr auto operator/(const LHS_T& lhs, const RHS_T& rhs) noexcept ->
    typename std::enable_if<is_quantity_compatible<LHS_T, RHS_T>::value && (offset_value_of<LHS_T>::value == 0) &&
                                (offset_value_of<RHS_T>::value == 0),
                            typename LHS_T::rep>::type {
    using result_type  = common_type_t<LHS_T, RHS_T>;
    const auto divisor = unit_cast<result_type>(rhs).count();
    ZTL_EXPECTS(divisor != 0);
    return details::divide_reps<typename LHS_T::rep>(unit_cast<result_type>(lhs).count(), divisor);
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

// The result unit is the product of both ratios, so the counts multiply directly.
template <typename LHS_T, typename RHS_T,
          typename std::enable_if<is_cross_quantity_multiply_compatible<LHS_T, RHS_T>::value, int>::type = 0>
constexpr auto operator*(const LHS_T& lhs, const RHS_T& rhs) noexcept
    -> basic_unit<typename LHS_T::rep, details::unit_multiply_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>> {
    using result_type =
        basic_unit<typename LHS_T::rep, details::unit_multiply_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>>;
    return result_type(details::multiply_reps<typename LHS_T::rep>(lhs.count(), rhs.count()));
}

// The result unit is the quotient of both ratios, so the counts divide directly
// (truncated toward zero).
template <typename LHS_T, typename RHS_T,
          typename std::enable_if<is_cross_quantity_divide_compatible<LHS_T, RHS_T>::value, int>::type = 0>
constexpr auto operator/(const LHS_T& lhs, const RHS_T& rhs) noexcept
    -> basic_unit<typename LHS_T::rep, details::unit_divide_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>> {
    using result_type =
        basic_unit<typename LHS_T::rep, details::unit_divide_t<unit_trait_of_t<LHS_T>, unit_trait_of_t<RHS_T>>>;
    ZTL_EXPECTS(rhs.count() != 0);
    return result_type(details::divide_reps<typename LHS_T::rep>(lhs.count(), rhs.count()));
}

}} // namespace ztl::units

#endif // ZTL_UNITS_BASICUNIT_HPP
