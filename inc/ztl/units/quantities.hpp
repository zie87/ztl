#ifndef ZTL_UNITS_QUANTITIES_HPP
#define ZTL_UNITS_QUANTITIES_HPP

#include "ztl/narrow.hpp"

namespace ztl {
namespace units {

// ============================================================================
//  Part 13 — Physical quantity tags
// ============================================================================

struct duration_tag {};    // seconds
struct voltage_tag {};     // volts
struct current_tag {};     // amperes
struct power_tag {};       // watts
struct resistance_tag {};  // ohms
struct pressure_tag {};    // pascals
struct temperature_tag {}; // kelvin
struct angle_tag {};       // turns
struct force_tag {};       // newtons
struct torque_tag {};      // newton-metres
struct distance_tag {};    // metres
struct speed_tag {};       // metres per second
struct gain_tag {};        // dimensionless (1/1)

// ============================================================================
//  Part 14 — Quantity algebra rules
// ============================================================================

// gain is the identity element ---
template <typename Q>
struct quantity_multiply<Q, gain_tag> {
    using type = Q;
};
template <typename Q>
struct quantity_multiply<gain_tag, Q> {
    using type = Q;
};
template <typename Q>
struct quantity_divide<Q, gain_tag> {
    using type = Q;
};
template <typename Q>
struct quantity_divide<Q, Q> {
    using type = gain_tag;
};
template <>
struct quantity_multiply<gain_tag, gain_tag> {
    using type = gain_tag;
};
template <>
struct quantity_divide<gain_tag, gain_tag> {
    using type = gain_tag;
};

// electrical ---
template <>
struct quantity_multiply<current_tag, voltage_tag> {
    using type = power_tag;
};
template <>
struct quantity_multiply<voltage_tag, current_tag> {
    using type = power_tag;
};
template <>
struct quantity_divide<voltage_tag, current_tag> {
    using type = resistance_tag;
};
template <>
struct quantity_multiply<current_tag, resistance_tag> {
    using type = voltage_tag;
};
template <>
struct quantity_multiply<resistance_tag, current_tag> {
    using type = voltage_tag;
};
template <>
struct quantity_divide<voltage_tag, resistance_tag> {
    using type = current_tag;
};
template <>
struct quantity_divide<power_tag, voltage_tag> {
    using type = current_tag;
};
template <>
struct quantity_divide<power_tag, current_tag> {
    using type = voltage_tag;
};

// mechanical ---
template <>
struct quantity_multiply<force_tag, distance_tag> {
    using type = torque_tag;
};
template <>
struct quantity_divide<distance_tag, duration_tag> {
    using type = speed_tag;
};
template <>
struct quantity_multiply<speed_tag, duration_tag> {
    using type = distance_tag;
};
template <>
struct quantity_multiply<duration_tag, speed_tag> {
    using type = distance_tag;
};
template <>
struct quantity_divide<distance_tag, speed_tag> {
    using type = duration_tag;
};
template <>
struct quantity_multiply<distance_tag, force_tag> {
    using type = torque_tag;
};
template <>
struct quantity_divide<torque_tag, distance_tag> {
    using type = force_tag;
};
template <>
struct quantity_divide<torque_tag, force_tag> {
    using type = distance_tag;
};

// ============================================================================
//  Part 15 — Standard unit aliases
// ============================================================================

namespace traits {

} // namespace traits

// --- Duration (base: second) ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using duration = basic_unit<REP, unit_trait<duration_tag, SCALING, OFFSET>>;

using hour        = duration<int, std::ratio<3600, 1>>;
using minute      = duration<int, std::ratio<60, 1>>;
using second      = duration<int>;
using millisecond = duration<int, std::milli>;
using microsecond = duration<int, std::micro>;
using nanosecond  = duration<int, std::nano>;

// --- Speed (base: metre per second) ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using speed = basic_unit<REP, unit_trait<speed_tag, SCALING, OFFSET>>;

using meter_per_second   = speed<int>;
using metre_per_second   = meter_per_second;
using kilometer_per_hour = speed<int, std::ratio<5, 18>>;
using kilometre_per_hour = kilometer_per_hour;

// --- Voltage ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using voltage = basic_unit<REP, unit_trait<voltage_tag, SCALING, OFFSET>>;

using kilovolt  = voltage<int, std::kilo>;
using volt      = voltage<int>;
using millivolt = voltage<int, std::milli>;
using microvolt = voltage<int, std::micro>;
using nanovolt  = voltage<int, std::nano>;
using picovolt  = voltage<int, std::pico>;

// --- Current ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using current = basic_unit<REP, unit_trait<current_tag, SCALING, OFFSET>>;

using kiloampere  = current<int, std::kilo>;
using ampere      = current<int>;
using milliampere = current<int, std::milli>;
using microampere = current<int, std::micro>;
using nanoampere  = current<int, std::nano>;
using picoampere  = current<int, std::pico>;

// --- Power ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using power = basic_unit<REP, unit_trait<power_tag, SCALING, OFFSET>>;

using kilowatt  = power<int, std::kilo>;
using watt      = power<int>;
using milliwatt = power<int, std::milli>;
using microwatt = power<int, std::micro>;
using nanowatt  = power<int, std::nano>;

// --- Resistance ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using resistance = basic_unit<REP, unit_trait<resistance_tag, SCALING, OFFSET>>;

using megaohm  = resistance<int, std::mega>;
using kiloohm  = resistance<int, std::kilo>;
using ohm      = resistance<int>;
using milliohm = resistance<int, std::milli>;
using microohm = resistance<int, std::micro>;

// --- Pressure ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using pressure = basic_unit<REP, unit_trait<pressure_tag, SCALING, OFFSET>>;

using pascal = pressure<int>;
// Conventional mmHg is 133.322387415 Pa. This int-friendly ratio keeps
// 760 mmHg == 101325 Pa after truncation while avoiding large common-type counts.
using millimeter_of_mercury = pressure<int, std::ratio<166653, 1250>>;
using millimetre_of_mercury = millimeter_of_mercury;

// --- Temperature (base: Kelvin) ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using temperature = basic_unit<REP, unit_trait<temperature_tag, SCALING, OFFSET>>;

using kelvin      = temperature<int>;
using millikelvin = temperature<int, std::milli>;
using microkelvin = temperature<int, std::micro>;

// Celsius: 0 °C = 273.15 K  →  offset = -273150 mK from absolute zero
using celsius      = temperature<int, std::ratio<1, 1>, offset_trait<-273150, std::milli>>;
using millicelsius = temperature<int, std::milli, offset_trait<-273150, std::milli>>;
using microcelsius = temperature<int, std::micro, offset_trait<-273150, std::milli>>;

// Fahrenheit: 0 °F = 255.3722… K
//   T[K] = T[°F] × 5/9 + 255.3722…
//   offset = -273.15 K + 32 × 5/9 K = -2298350/9000 K
using fahrenheit = temperature<int, std::ratio<5, 9>, offset_trait<-2298350, std::ratio<1, 9000>>>;

// --- Angle(base: turns) ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using angle = basic_unit<REP, unit_trait<angle_tag, SCALING, OFFSET>>;

using turn = angle<int>;
// Radian: 1 turn = 2π rad  →  ratio = 1e9 / (2 × 3141592654)
//   Kept as integer ratio; precision ≈ 0.3 ppb.
using radian_per_turn = std::ratio<1000000000, 6283185307>;
using radian          = angle<int, radian_per_turn>;
using milliradian     = angle<int, std::ratio_multiply<radian_per_turn, std::milli>>;
using microradian     = angle<int, std::ratio_multiply<radian_per_turn, std::micro>>;

// Degree: 1 turn = 360°
using degree_per_turn = std::ratio<1, 360>;
using degree          = angle<int, degree_per_turn>;
using millidegree     = angle<int, std::ratio_multiply<degree_per_turn, std::milli>>;
using microdegree     = angle<int, std::ratio_multiply<degree_per_turn, std::micro>>;

// Gradian: 1 turn = 400 gon
using gradian_per_turn = std::ratio<1, 400>;
using gradian          = angle<int, gradian_per_turn>;
using milligradian     = angle<int, std::ratio_multiply<gradian_per_turn, std::milli>>;
using microgradian     = angle<int, std::ratio_multiply<gradian_per_turn, std::micro>>;

// --- Force ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using force = basic_unit<REP, unit_trait<force_tag, SCALING, OFFSET>>;

using kilonewton  = force<int, std::kilo>;
using newton      = force<int>;
using millinewton = force<int, std::milli>;
using micronewton = force<int, std::micro>;

// --- Torque ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using torque = basic_unit<REP, unit_trait<torque_tag, SCALING, OFFSET>>;

using kilonewton_meter  = torque<int, std::kilo>;
using newton_meter      = torque<int>;
using newton_centimeter = torque<int, std::centi>;
using newton_millimeter = torque<int, std::milli>;

// --- Distance ---
template <typename REP, typename SCALING = std::ratio<1, 1>, typename OFFSET = offset_trait<>>
using distance = basic_unit<REP, unit_trait<distance_tag, SCALING, OFFSET>>;

using kilometer  = distance<int, std::kilo>;
using meter      = distance<int>;
using centimeter = distance<int, std::centi>;
using millimeter = distance<int, std::milli>;
using micrometer = distance<int, std::micro>;
using nanometer  = distance<int, std::nano>;
} // namespace units

inline namespace literals {
inline namespace duration_literals {
constexpr ::ztl::units::hour operator""_h(unsigned long long v) {
    using rep_type = ::ztl::units::hour::rep;
    return ::ztl::units::hour(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::minute operator""_min(unsigned long long v) {
    using rep_type = ::ztl::units::minute::rep;
    return ::ztl::units::minute(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::second operator""_s(unsigned long long v) {
    using rep_type = ::ztl::units::second::rep;
    return ::ztl::units::second(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millisecond operator""_ms(unsigned long long v) {
    using rep_type = ::ztl::units::millisecond::rep;
    return ::ztl::units::millisecond(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microsecond operator""_us(unsigned long long v) {
    using rep_type = ::ztl::units::microsecond::rep;
    return ::ztl::units::microsecond(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::nanosecond operator""_ns(unsigned long long v) {
    using rep_type = ::ztl::units::nanosecond::rep;
    return ::ztl::units::nanosecond(::ztl::narrow<rep_type>(v));
}
} // namespace duration_literals
inline namespace speed_literals {
constexpr ::ztl::units::meter_per_second operator""_mps(unsigned long long v) {
    using rep_type = ::ztl::units::meter_per_second::rep;
    return ::ztl::units::meter_per_second(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::kilometer_per_hour operator""_kmh(unsigned long long v) {
    using rep_type = ::ztl::units::kilometer_per_hour::rep;
    return ::ztl::units::kilometer_per_hour(::ztl::narrow<rep_type>(v));
}
} // namespace speed_literals
inline namespace voltage_literals {
constexpr ::ztl::units::kilovolt operator""_kV(unsigned long long v) {
    using rep_type = ::ztl::units::kilovolt::rep;
    return ::ztl::units::kilovolt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::volt operator""_V(unsigned long long v) {
    using rep_type = ::ztl::units::volt::rep;
    return ::ztl::units::volt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millivolt operator""_mV(unsigned long long v) {
    using rep_type = ::ztl::units::millivolt::rep;
    return ::ztl::units::millivolt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microvolt operator""_uV(unsigned long long v) {
    using rep_type = ::ztl::units::microvolt::rep;
    return ::ztl::units::microvolt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::nanovolt operator""_nV(unsigned long long v) {
    using rep_type = ::ztl::units::nanovolt::rep;
    return ::ztl::units::nanovolt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::picovolt operator""_pV(unsigned long long v) {
    using rep_type = ::ztl::units::picovolt::rep;
    return ::ztl::units::picovolt(::ztl::narrow<rep_type>(v));
}
} // namespace voltage_literals
inline namespace current_literals {
constexpr ::ztl::units::kiloampere operator""_kA(unsigned long long v) {
    using rep_type = ::ztl::units::kiloampere::rep;
    return ::ztl::units::kiloampere(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::ampere operator""_A(unsigned long long v) {
    using rep_type = ::ztl::units::ampere::rep;
    return ::ztl::units::ampere(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::milliampere operator""_mA(unsigned long long v) {
    using rep_type = ::ztl::units::milliampere::rep;
    return ::ztl::units::milliampere(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microampere operator""_uA(unsigned long long v) {
    using rep_type = ::ztl::units::microampere::rep;
    return ::ztl::units::microampere(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::nanoampere operator""_nA(unsigned long long v) {
    using rep_type = ::ztl::units::nanoampere::rep;
    return ::ztl::units::nanoampere(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::picoampere operator""_pA(unsigned long long v) {
    using rep_type = ::ztl::units::picoampere::rep;
    return ::ztl::units::picoampere(::ztl::narrow<rep_type>(v));
}
} // namespace current_literals
inline namespace power_literals {
constexpr ::ztl::units::kilowatt operator""_kW(unsigned long long v) {
    using rep_type = ::ztl::units::kilowatt::rep;
    return ::ztl::units::kilowatt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::watt operator""_W(unsigned long long v) {
    using rep_type = ::ztl::units::watt::rep;
    return ::ztl::units::watt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::milliwatt operator""_mW(unsigned long long v) {
    using rep_type = ::ztl::units::milliwatt::rep;
    return ::ztl::units::milliwatt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microwatt operator""_uW(unsigned long long v) {
    using rep_type = ::ztl::units::microwatt::rep;
    return ::ztl::units::microwatt(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::nanowatt operator""_nW(unsigned long long v) {
    using rep_type = ::ztl::units::nanowatt::rep;
    return ::ztl::units::nanowatt(::ztl::narrow<rep_type>(v));
}
} // namespace power_literals
inline namespace resistance_literals {
constexpr ::ztl::units::megaohm operator""_MOhm(unsigned long long v) {
    using rep_type = ::ztl::units::megaohm::rep;
    return ::ztl::units::megaohm(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::kiloohm operator""_kOhm(unsigned long long v) {
    using rep_type = ::ztl::units::kiloohm::rep;
    return ::ztl::units::kiloohm(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::ohm operator""_Ohm(unsigned long long v) {
    using rep_type = ::ztl::units::ohm::rep;
    return ::ztl::units::ohm(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::milliohm operator""_mOhm(unsigned long long v) {
    using rep_type = ::ztl::units::milliohm::rep;
    return ::ztl::units::milliohm(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microohm operator""_uOhm(unsigned long long v) {
    using rep_type = ::ztl::units::microohm::rep;
    return ::ztl::units::microohm(::ztl::narrow<rep_type>(v));
}
} // namespace resistance_literals
inline namespace pressure_literals {
constexpr ::ztl::units::pascal operator""_Pa(unsigned long long v) {
    using rep_type = ::ztl::units::pascal::rep;
    return ::ztl::units::pascal(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millimeter_of_mercury operator""_mmHg(unsigned long long v) {
    using rep_type = ::ztl::units::millimeter_of_mercury::rep;
    return ::ztl::units::millimeter_of_mercury(::ztl::narrow<rep_type>(v));
}
} // namespace pressure_literals
inline namespace temperature_literals {

constexpr ::ztl::units::kelvin operator""_K(unsigned long long v) {
    using rep_type = ::ztl::units::kelvin::rep;
    return ::ztl::units::kelvin(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millikelvin operator""_mK(unsigned long long v) {
    using rep_type = ::ztl::units::millikelvin::rep;
    return ::ztl::units::millikelvin(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microkelvin operator""_uK(unsigned long long v) {
    using rep_type = ::ztl::units::microkelvin::rep;
    return ::ztl::units::microkelvin(::ztl::narrow<rep_type>(v));
}

constexpr ::ztl::units::celsius operator""_C(unsigned long long v) {
    using rep_type = ::ztl::units::celsius::rep;
    return ::ztl::units::celsius(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millicelsius operator""_mC(unsigned long long v) {
    using rep_type = ::ztl::units::millicelsius::rep;
    return ::ztl::units::millicelsius(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microcelsius operator""_uC(unsigned long long v) {
    using rep_type = ::ztl::units::microcelsius::rep;
    return ::ztl::units::microcelsius(::ztl::narrow<rep_type>(v));
}

constexpr ::ztl::units::fahrenheit operator""_F(unsigned long long v) {
    using rep_type = ::ztl::units::fahrenheit::rep;
    return ::ztl::units::fahrenheit(::ztl::narrow<rep_type>(v));
}
} // namespace temperature_literals
inline namespace angle_literals {
constexpr ::ztl::units::radian operator""_Rad(unsigned long long v) {
    using rep_type = ::ztl::units::radian::rep;
    return ::ztl::units::radian(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::milliradian operator""_mRad(unsigned long long v) {
    using rep_type = ::ztl::units::milliradian::rep;
    return ::ztl::units::milliradian(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microradian operator""_uRad(unsigned long long v) {
    using rep_type = ::ztl::units::microradian::rep;
    return ::ztl::units::microradian(::ztl::narrow<rep_type>(v));
}

constexpr ::ztl::units::degree operator""_Deg(unsigned long long v) {
    using rep_type = ::ztl::units::degree::rep;
    return ::ztl::units::degree(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millidegree operator""_mDeg(unsigned long long v) {
    using rep_type = ::ztl::units::millidegree::rep;
    return ::ztl::units::millidegree(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microdegree operator""_uDeg(unsigned long long v) {
    using rep_type = ::ztl::units::microdegree::rep;
    return ::ztl::units::microdegree(::ztl::narrow<rep_type>(v));
}

constexpr ::ztl::units::gradian operator""_Gon(unsigned long long v) {
    using rep_type = ::ztl::units::gradian::rep;
    return ::ztl::units::gradian(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::milligradian operator""_mGon(unsigned long long v) {
    using rep_type = ::ztl::units::milligradian::rep;
    return ::ztl::units::milligradian(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::microgradian operator""_uGon(unsigned long long v) {
    using rep_type = ::ztl::units::microgradian::rep;
    return ::ztl::units::microgradian(::ztl::narrow<rep_type>(v));
}
} // namespace angle_literals
inline namespace force_literals {
constexpr ::ztl::units::kilonewton operator""_kN(unsigned long long v) {
    using rep_type = ::ztl::units::kilonewton::rep;
    return ::ztl::units::kilonewton(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::newton operator""_N(unsigned long long v) {
    using rep_type = ::ztl::units::newton::rep;
    return ::ztl::units::newton(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millinewton operator""_mN(unsigned long long v) {
    using rep_type = ::ztl::units::millinewton::rep;
    return ::ztl::units::millinewton(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::micronewton operator""_uN(unsigned long long v) {
    using rep_type = ::ztl::units::micronewton::rep;
    return ::ztl::units::micronewton(::ztl::narrow<rep_type>(v));
}
} // namespace force_literals
inline namespace torque_literals {
constexpr ::ztl::units::kilonewton_meter operator""_kNm(unsigned long long v) {
    using rep_type = ::ztl::units::kilonewton_meter::rep;
    return ::ztl::units::kilonewton_meter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::newton_meter operator""_Nm(unsigned long long v) {
    using rep_type = ::ztl::units::newton_meter::rep;
    return ::ztl::units::newton_meter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::newton_centimeter operator""_Ncm(unsigned long long v) {
    using rep_type = ::ztl::units::newton_centimeter::rep;
    return ::ztl::units::newton_centimeter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::newton_millimeter operator""_Nmm(unsigned long long v) {
    using rep_type = ::ztl::units::newton_millimeter::rep;
    return ::ztl::units::newton_millimeter(::ztl::narrow<rep_type>(v));
}
} // namespace torque_literals
inline namespace distance_literals {
constexpr ::ztl::units::kilometer operator""_km(unsigned long long v) {
    using rep_type = ::ztl::units::kilometer::rep;
    return ::ztl::units::kilometer(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::meter operator""_m(unsigned long long v) {
    using rep_type = ::ztl::units::meter::rep;
    return ::ztl::units::meter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::centimeter operator""_cm(unsigned long long v) {
    using rep_type = ::ztl::units::centimeter::rep;
    return ::ztl::units::centimeter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::millimeter operator""_mm(unsigned long long v) {
    using rep_type = ::ztl::units::millimeter::rep;
    return ::ztl::units::millimeter(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::micrometer operator""_um(unsigned long long v) {
    using rep_type = ::ztl::units::micrometer::rep;
    return ::ztl::units::micrometer(::ztl::narrow<rep_type>(v));
}
constexpr ::ztl::units::nanometer operator""_nm(unsigned long long v) {
    using rep_type = ::ztl::units::nanometer::rep;
    return ::ztl::units::nanometer(::ztl::narrow<rep_type>(v));
}
} // namespace distance_literals
} // namespace literals

namespace units {
using namespace ::ztl::literals::duration_literals;
using namespace ::ztl::literals::speed_literals;
using namespace ::ztl::literals::voltage_literals;
using namespace ::ztl::literals::current_literals;
using namespace ::ztl::literals::power_literals;
using namespace ::ztl::literals::resistance_literals;
using namespace ::ztl::literals::pressure_literals;
using namespace ::ztl::literals::temperature_literals;
using namespace ::ztl::literals::angle_literals;
using namespace ::ztl::literals::force_literals;
using namespace ::ztl::literals::torque_literals;
using namespace ::ztl::literals::distance_literals;
} // namespace units

} // namespace ztl

#endif // ZTL_UNITS_QUANTITIES_HPP
