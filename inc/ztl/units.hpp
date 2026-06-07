// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UNITS_HPP
#define ZTL_UNITS_HPP

#include "ztl/units/ratio.hpp"
#include "ztl/units/unit_trait.hpp"
#include "ztl/units/basic_unit.hpp"
#include "ztl/units/quantities.hpp"

#include <chrono>
#include <type_traits>

namespace ztl { namespace units {

namespace Values {

using h_t   = hour;
using min_t = minute;
using s_t   = second;
using ms_t  = millisecond;
using us_t  = microsecond;
using ns_t  = nanosecond;

using mps_t = meter_per_second;
using kmh_t = kilometer_per_hour;

using kV_t = kilovolt;
using V_t  = volt;
using mV_t = millivolt;
using uV_t = microvolt;
using nV_t = nanovolt;
using pV_t = picovolt;

using kA_t = kiloampere;
using A_t  = ampere;
using mA_t = milliampere;
using uA_t = microampere;
using nA_t = nanoampere;
using pA_t = picoampere;

using kW_t = kilowatt;
using W_t  = watt;
using mW_t = milliwatt;
using uW_t = microwatt;
using nW_t = nanowatt;

using MOhm_t = megaohm;
using kOhm_t = kiloohm;
using Ohm_t  = ohm;
using mOhm_t = milliohm;
using uOhm_t = microohm;

using Pa_t   = pascal;
using mmHg_t = millimeter_of_mercury;

using K_t  = kelvin;
using mK_t = millikelvin;
using uK_t = microkelvin;

using C_t  = celsius;
using mC_t = millicelsius;
using uC_t = microcelsius;

using F_t = fahrenheit;

using kN_t = kilonewton;
using N_t  = newton;
using mN_t = millinewton;
using uN_t = micronewton;

using kNm_t = kilonewton_meter;
using Nm_t  = newton_meter;
using Ncm_t = newton_centimeter;
using Nmm_t = newton_millimeter;

using Rad_t  = radian;
using mRad_t = milliradian;
using uRad_t = microradian;

using Deg_t  = degree;
using mDeg_t = millidegree;
using uDeg_t = microdegree;

using Gon_t  = gradian;
using mGon_t = milligradian;
using uGon_t = microgradian;

using km_t = kilometer;
using m_t  = meter;
using cm_t = centimeter;
using mm_t = millimeter;
using um_t = micrometer;
using nm_t = nanometer;

} // namespace Values

// ============================================================================
//  Part 19 — Unity constants  (multiplier-free unit references: 1 V, 1 mA, …)
// ============================================================================
namespace Unity {

static constexpr Values::h_t   h(1);
static constexpr Values::min_t min(1);
static constexpr Values::s_t   s(1);
static constexpr Values::ms_t  ms(1);
static constexpr Values::us_t  us(1);
static constexpr Values::ns_t  ns(1);

static constexpr Values::mps_t mps(1);
static constexpr Values::kmh_t kmh(1);

static constexpr Values::kV_t kV(1);
static constexpr Values::V_t  V(1);
static constexpr Values::mV_t mV(1);
static constexpr Values::uV_t uV(1);
static constexpr Values::nV_t nV(1);
static constexpr Values::pV_t pV(1);

static constexpr Values::kA_t kA(1);
static constexpr Values::A_t  A(1);
static constexpr Values::mA_t mA(1);
static constexpr Values::uA_t uA(1);
static constexpr Values::nA_t nA(1);
static constexpr Values::pA_t pA(1);

static constexpr Values::kW_t kW(1);
static constexpr Values::W_t  W(1);
static constexpr Values::mW_t mW(1);
static constexpr Values::uW_t uW(1);
static constexpr Values::nW_t nW(1);

static constexpr Values::MOhm_t MOhm(1);
static constexpr Values::kOhm_t kOhm(1);
static constexpr Values::Ohm_t  Ohm(1);
static constexpr Values::mOhm_t mOhm(1);
static constexpr Values::uOhm_t uOhm(1);

static constexpr Values::Pa_t   Pa(1);
static constexpr Values::mmHg_t mmHg(1);

static constexpr Values::K_t  K(1);
static constexpr Values::mK_t mK(1);
static constexpr Values::uK_t uK(1);

static constexpr Values::C_t  C(1);
static constexpr Values::mC_t mC(1);
static constexpr Values::uC_t uC(1);

static constexpr Values::F_t F(1);

static constexpr Values::kN_t kN(1);
static constexpr Values::N_t  N(1);
static constexpr Values::mN_t mN(1);
static constexpr Values::uN_t uN(1);

static constexpr Values::Rad_t  Rad(1);
static constexpr Values::mRad_t mRad(1);
static constexpr Values::uRad_t uRad(1);

static constexpr Values::Deg_t  Deg(1);
static constexpr Values::mDeg_t mDeg(1);
static constexpr Values::uDeg_t uDeg(1);

static constexpr Values::Gon_t  Gon(1);
static constexpr Values::mGon_t mGon(1);
static constexpr Values::uGon_t uGon(1);

static constexpr Values::kNm_t kNm(1);
static constexpr Values::Nm_t  Nm(1);
static constexpr Values::Ncm_t Ncm(1);
static constexpr Values::Nmm_t Nmm(1);

static constexpr Values::km_t km(1);
static constexpr Values::m_t  m(1);
static constexpr Values::cm_t cm(1);
static constexpr Values::mm_t mm(1);
static constexpr Values::um_t um(1);
static constexpr Values::nm_t nm(1);

} // namespace Unity

namespace details {

template <typename T>
struct is_chrono_duration : std::false_type {};

template <typename REP, typename PERIOD>
struct is_chrono_duration<std::chrono::duration<REP, PERIOD>> : std::true_type {};

} // namespace details

template <typename UNIT_T, typename = void>
struct is_duration_unit : std::false_type {};

template <typename UNIT_T>
struct is_duration_unit<UNIT_T, typename std::enable_if<is_unit<UNIT_T>::value &&
                                                        std::is_same<quantity_of_t<UNIT_T>, duration_tag>::value>::type>
    : std::true_type {};

template <typename TARGET_T, typename SOURCE_T>
constexpr auto unit_cast(const SOURCE_T& value) ->
    typename std::enable_if<details::is_chrono_duration<TARGET_T>::value && is_duration_unit<SOURCE_T>::value,
                            TARGET_T>::type {
    using source_duration = std::chrono::duration<typename SOURCE_T::rep, ratio_of_t<SOURCE_T>>;
    return std::chrono::duration_cast<TARGET_T>(source_duration(value.count()));
}

template <typename TARGET_T, typename REP, typename PERIOD>
constexpr auto unit_cast(const std::chrono::duration<REP, PERIOD>& value) ->
    typename std::enable_if<is_duration_unit<TARGET_T>::value, TARGET_T>::type {
    using target_duration = std::chrono::duration<typename TARGET_T::rep, ratio_of_t<TARGET_T>>;
    return TARGET_T(static_cast<typename TARGET_T::rep>(std::chrono::duration_cast<target_duration>(value).count()));
}

template <typename CHRONO_DURATION, typename UNIT_T>
constexpr auto to_chrono_duration(const UNIT_T& value) ->
    typename std::enable_if<details::is_chrono_duration<CHRONO_DURATION>::value && is_duration_unit<UNIT_T>::value,
                            CHRONO_DURATION>::type {
    return unit_cast<CHRONO_DURATION>(value);
}

template <typename UNIT_T, typename CHRONO_DURATION>
constexpr auto from_chrono_duration(const CHRONO_DURATION& value) ->
    typename std::enable_if<is_duration_unit<UNIT_T>::value && details::is_chrono_duration<CHRONO_DURATION>::value,
                            UNIT_T>::type {
    return unit_cast<UNIT_T>(value);
}

}} // namespace ztl::units

#endif // ZTL_UNITS_HPP
