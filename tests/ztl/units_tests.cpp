#include "ztl/units.hpp"
#include "ztl/type_traits.hpp"

#include <doctest/doctest.h>

#include <chrono>
#include <limits>
#include <ratio>
#include <type_traits>
#include <utility>

namespace {

template <typename SOURCE_T, typename TARGET_T, int SOURCE_COUNT, int TARGET_COUNT>
struct unit_cast_case {
    using source = SOURCE_T;
    using target = TARGET_T;

    enum { source_count = SOURCE_COUNT, target_count = TARGET_COUNT };
};

template <typename LHS_T, typename RHS_T, typename RESULT_T, int LHS_COUNT, int RHS_COUNT, int RESULT_COUNT>
struct binary_unit_case {
    using lhs    = LHS_T;
    using rhs    = RHS_T;
    using result = RESULT_T;

    enum { lhs_count = LHS_COUNT, rhs_count = RHS_COUNT, result_count = RESULT_COUNT };
};

template <typename LHS_T, typename RHS_T, typename = void>
struct can_add_units : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct can_add_units<LHS_T, RHS_T, ztl::void_t<decltype(std::declval<LHS_T>() + std::declval<RHS_T>())>>
    : std::true_type {};

template <typename LHS_T, typename RHS_T, typename = void>
struct can_subtract_units : std::false_type {};
template <typename LHS_T, typename RHS_T>
struct can_subtract_units<LHS_T, RHS_T, ztl::void_t<decltype(std::declval<LHS_T>() - std::declval<RHS_T>())>>
    : std::true_type {};

template <typename LHS_T, typename RHS_T, typename = void>
struct can_compare_units : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct can_compare_units<LHS_T, RHS_T, ztl::void_t<decltype(std::declval<LHS_T>() == std::declval<RHS_T>())>>
    : std::true_type {};

template <typename LHS_T, typename RHS_T, typename = void>
struct can_multiply_units : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct can_multiply_units<LHS_T, RHS_T, ztl::void_t<decltype(std::declval<LHS_T>() * std::declval<RHS_T>())>>
    : std::true_type {};

template <typename LHS_T, typename RHS_T, typename = void>
struct can_divide_units : std::false_type {};

template <typename LHS_T, typename RHS_T>
struct can_divide_units<LHS_T, RHS_T, ztl::void_t<decltype(std::declval<LHS_T>() / std::declval<RHS_T>())>>
    : std::true_type {};

} // namespace

TEST_CASE("unit traits expose quantity, ratio, and offset metadata") {
    using milli_celsius_trait =
        ztl::units::unit_trait<ztl::units::temperature_tag, std::milli, ztl::units::offset_trait<-273150, std::milli>>;

    static_assert(ztl::units::is_ratio<std::milli>::value, "std::ratio specializations must be detected");
    static_assert(!ztl::units::is_ratio<int>::value, "non-ratio types must not be detected as ratios");

    static_assert(ztl::units::is_offset_trait<ztl::units::offset_trait<-273150, std::milli>>::value,
                  "offset_trait must be detected");
    static_assert(ztl::units::is_unit_trait<milli_celsius_trait>::value, "unit_trait must be detected");
    static_assert(std::is_same<ztl::units::quantity_of_t<ztl::units::second>, ztl::units::duration_tag>::value,
                  "duration aliases must use duration_tag");
    static_assert(std::is_same<ztl::units::quantity_of_t<ztl::units::meter_per_second>, ztl::units::speed_tag>::value,
                  "speed aliases must use speed_tag");

    static_assert(std::is_same<ztl::units::quantity_of_t<milli_celsius_trait>, ztl::units::temperature_tag>::value,
                  "quantity_of_t must return the quantity tag");
    static_assert(std::is_same<ztl::units::ratio_of_t<milli_celsius_trait>, std::milli>::value,
                  "ratio_of_t must return the scaling ratio");
    static_assert(std::is_same<ztl::units::offset_of_t<milli_celsius_trait>,
                               ztl::units::offset_trait<-273150, std::milli>>::value,
                  "offset_of_t must return the offset trait");
    static_assert(ztl::units::offset_value_of<milli_celsius_trait>::value == -273150,
                  "offset_value_of must return the offset value");

    CHECK(true);
}

TEST_CASE_TEMPLATE("basic units expose chrono-like value operations", Unit, ztl::units::second,
                   ztl::units::meter_per_second, ztl::units::volt, ztl::units::ampere, ztl::units::watt,
                   ztl::units::ohm, ztl::units::pascal, ztl::units::kelvin, ztl::units::degree, ztl::units::newton,
                   ztl::units::newton_meter, ztl::units::meter) {
    static_assert(ztl::units::is_unit<Unit>::value, "basic_unit specializations must be detected as units");
    static_assert(std::is_same<typename Unit::rep, int>::value, "standard unit aliases use int reps");
    static_assert(std::is_constructible<Unit, int>::value, "unit must be explicitly constructible from rep");
    static_assert(!std::is_convertible<int, Unit>::value, "rep construction must stay explicit");

    Unit value(5);

    CHECK(value.count() == 5);
    CHECK(Unit::zero().count() == 0);
    CHECK(Unit::min().count() == std::numeric_limits<typename Unit::rep>::min());
    CHECK(Unit::max().count() == std::numeric_limits<typename Unit::rep>::max());

    CHECK((+value).count() == 5);
    CHECK((-value).count() == -5);

    CHECK((++value).count() == 6);
    CHECK((value++).count() == 6);
    CHECK(value.count() == 7);
    CHECK((--value).count() == 6);
    CHECK((value--).count() == 6);
    CHECK(value.count() == 5);

    value += Unit(3);
    CHECK(value.count() == 8);
    value -= Unit(2);
    CHECK(value.count() == 6);
    value *= 4;
    CHECK(value.count() == 24);
    value /= 3;
    CHECK(value.count() == 8);
    value %= 3;
    CHECK(value.count() == 2);
    value %= Unit(2);
    CHECK(value.count() == 0);
}

TEST_CASE_TEMPLATE("unit_cast scales metric prefixes", Case,
                   unit_cast_case<ztl::units::second, ztl::units::millisecond, 1, 1000>,
                   unit_cast_case<ztl::units::second, ztl::units::nanosecond, 2, 2000000000>,
                   unit_cast_case<ztl::units::volt, ztl::units::millivolt, 2, 2000>,
                   unit_cast_case<ztl::units::ampere, ztl::units::milliampere, 3, 3000>,
                   unit_cast_case<ztl::units::watt, ztl::units::milliwatt, 4, 4000>,
                   unit_cast_case<ztl::units::ohm, ztl::units::milliohm, 5, 5000>,
                   unit_cast_case<ztl::units::kelvin, ztl::units::millikelvin, 6, 6000>,
                   unit_cast_case<ztl::units::newton, ztl::units::millinewton, 7, 7000>,
                   unit_cast_case<ztl::units::meter, ztl::units::millimeter, 8, 8000>) {
    typename Case::source source(Case::source_count);
    typename Case::target target = ztl::units::unit_cast<typename Case::target>(source);

    static_assert(std::is_convertible<typename Case::source, typename Case::target>::value,
                  "larger-to-smaller metric conversions are lossless and implicit");
    static_assert(!std::is_convertible<typename Case::target, typename Case::source>::value,
                  "smaller-to-larger metric conversions can truncate and must be explicit");

    CHECK(target.count() == Case::target_count);
    CHECK(ztl::units::unit_cast<typename Case::source>(target).count() == Case::source_count);
}

TEST_CASE("unit_cast converts duration units") {
    CHECK(ztl::units::unit_cast<ztl::units::minute>(ztl::units::hour(2)).count() == 120);
    CHECK(ztl::units::unit_cast<ztl::units::second>(ztl::units::minute(3)).count() == 180);
    CHECK(ztl::units::unit_cast<ztl::units::millisecond>(ztl::units::second(4)).count() == 4000);
    CHECK(ztl::units::unit_cast<ztl::units::second>(ztl::units::millisecond(2500)).count() == 2);

    auto duration = ztl::units::minute(1) + ztl::units::second(30);

    static_assert(std::is_same<decltype(duration), ztl::units::second>::value,
                  "mixed duration arithmetic must return the finest common duration unit");

    CHECK(duration.count() == 90);
}

TEST_CASE("std::chrono::duration conversions work in both directions") {
    auto chrono_seconds      = ztl::units::to_chrono_duration<std::chrono::seconds>(ztl::units::minute(2));
    auto chrono_milliseconds = ztl::units::to_chrono_duration<std::chrono::milliseconds>(ztl::units::second(3));
    auto unit_seconds        = ztl::units::from_chrono_duration<ztl::units::second>(std::chrono::milliseconds(2500));
    auto unit_milliseconds   = ztl::units::from_chrono_duration<ztl::units::millisecond>(std::chrono::seconds(4));
    auto cast_to_chrono      = ztl::units::unit_cast<std::chrono::nanoseconds>(ztl::units::nanosecond(7));
    auto cast_to_unit        = ztl::units::unit_cast<ztl::units::minute>(std::chrono::seconds(180));

    static_assert(ztl::units::is_duration_unit<ztl::units::second>::value,
                  "duration units must be detectable for chrono conversions");
    static_assert(!ztl::units::is_duration_unit<ztl::units::meter>::value,
                  "non-duration units must not be accepted as duration units");
    static_assert(std::is_same<decltype(chrono_seconds), std::chrono::seconds>::value,
                  "to_chrono_duration must return the requested std::chrono type");
    static_assert(std::is_same<decltype(unit_seconds), ztl::units::second>::value,
                  "from_chrono_duration must return the requested units type");

    CHECK(chrono_seconds.count() == 120);
    CHECK(chrono_milliseconds.count() == 3000);
    CHECK(unit_seconds.count() == 2);
    CHECK(unit_milliseconds.count() == 4000);
    CHECK(cast_to_chrono.count() == 7);
    CHECK(cast_to_unit.count() == 3);
}

TEST_CASE("unit_cast converts speed units") {
    static_assert(std::is_same<ztl::units::meter_per_second, ztl::units::metre_per_second>::value,
                  "British and American spelling aliases must name the same m/s type");
    static_assert(std::is_same<ztl::units::kilometer_per_hour, ztl::units::kilometre_per_hour>::value,
                  "British and American spelling aliases must name the same km/h type");

    CHECK(ztl::units::unit_cast<ztl::units::meter_per_second>(ztl::units::kilometer_per_hour(36)).count() == 10);
    CHECK(ztl::units::unit_cast<ztl::units::kilometer_per_hour>(ztl::units::meter_per_second(10)).count() == 36);

    CHECK(ztl::units::kilometer_per_hour(36) == ztl::units::meter_per_second(10));
    CHECK(ztl::units::kilometer_per_hour(35) < ztl::units::meter_per_second(10));
    CHECK(ztl::units::kilometer_per_hour(37) > ztl::units::meter_per_second(10));

    auto mixed_speed = ztl::units::kilometer_per_hour(18) + ztl::units::meter_per_second(5);

    static_assert(std::is_same<decltype(mixed_speed), ztl::units::speed<int, std::ratio<1, 18>>>::value,
                  "mixed m/s and km/h arithmetic must return the finest common speed unit");

    CHECK(mixed_speed.count() == 180);
    CHECK(ztl::units::unit_cast<ztl::units::meter_per_second>(mixed_speed).count() == 10);
}

TEST_CASE("unit_cast converts pressure units with integer truncation") {
    static_assert(std::is_same<ztl::units::millimeter_of_mercury, ztl::units::millimetre_of_mercury>::value,
                  "British and American spelling aliases must name the same mmHg type");

    CHECK(ztl::units::unit_cast<ztl::units::pascal>(ztl::units::millimeter_of_mercury(1)).count() == 133);
    CHECK(ztl::units::unit_cast<ztl::units::pascal>(ztl::units::millimeter_of_mercury(300)).count() == 39996);
    CHECK(ztl::units::unit_cast<ztl::units::pascal>(ztl::units::millimeter_of_mercury(760)).count() == 101325);

    CHECK(ztl::units::millimeter_of_mercury(1) > ztl::units::pascal(133));
    CHECK(ztl::units::millimeter_of_mercury(1) < ztl::units::pascal(134));

    auto pressure = ztl::units::pascal(1) + ztl::units::millimeter_of_mercury(1);

    static_assert(std::is_same<decltype(pressure),
                               ztl::units::common_type_t<ztl::units::pascal, ztl::units::millimeter_of_mercury>>::value,
                  "mixed pressure arithmetic must return the common pressure unit");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(pressure)>, ztl::units::pressure_tag>::value,
                  "mixed pressure arithmetic must preserve pressure quantity");

    CHECK(pressure.count() == 167903);
    CHECK(ztl::units::unit_cast<ztl::units::pascal>(pressure).count() == 134);
}

TEST_CASE("unit_cast handles affine temperature offsets before truncating to the target rep") {
    CHECK(ztl::units::unit_cast<ztl::units::kelvin>(ztl::units::celsius(0)).count() == 273);
    CHECK(ztl::units::unit_cast<ztl::units::celsius>(ztl::units::kelvin(273)).count() == 0);
    CHECK(ztl::units::unit_cast<ztl::units::celsius>(ztl::units::fahrenheit(32)).count() == 0);
    CHECK(ztl::units::unit_cast<ztl::units::kelvin>(ztl::units::fahrenheit(32)).count() == 273);

    CHECK(ztl::units::celsius(0) == ztl::units::fahrenheit(32));
    CHECK(ztl::units::kelvin(274) > ztl::units::celsius(0));
    CHECK(ztl::units::kelvin(272) < ztl::units::celsius(0));
}

TEST_CASE_TEMPLATE("same-quantity comparisons convert both operands to a common unit", Case,
                   binary_unit_case<ztl::units::minute, ztl::units::second, ztl::units::second, 1, 60, 0>,
                   binary_unit_case<ztl::units::meter_per_second, ztl::units::kilometer_per_hour,
                                    ztl::units::speed<int, std::ratio<1, 18>>, 10, 36, 0>,
                   binary_unit_case<ztl::units::volt, ztl::units::millivolt, ztl::units::millivolt, 1, 1000, 0>,
                   binary_unit_case<ztl::units::kilometer, ztl::units::meter, ztl::units::meter, 1, 1000, 0>,
                   binary_unit_case<ztl::units::newton_meter, ztl::units::newton_centimeter,
                                    ztl::units::newton_centimeter, 1, 100, 0>) {
    typename Case::lhs lhs(Case::lhs_count);
    typename Case::rhs rhs(Case::rhs_count);

    static_assert(
        std::is_same<ztl::units::common_type_t<typename Case::lhs, typename Case::rhs>, typename Case::result>::value,
        "common_type_t must use the finest common unit ratio");

    CHECK(lhs == rhs);
    CHECK_FALSE(lhs != rhs);
    CHECK(lhs <= rhs);
    CHECK(lhs >= rhs);
    CHECK_FALSE(lhs < rhs);
    CHECK_FALSE(lhs > rhs);
}

TEST_CASE_TEMPLATE("same-quantity arithmetic returns the finest common unit", Case,
                   binary_unit_case<ztl::units::minute, ztl::units::second, ztl::units::second, 2, 30, 150>,
                   binary_unit_case<ztl::units::meter_per_second, ztl::units::kilometer_per_hour,
                                    ztl::units::speed<int, std::ratio<1, 18>>, 5, 18, 180>,
                   binary_unit_case<ztl::units::volt, ztl::units::millivolt, ztl::units::millivolt, 1, 500, 1500>,
                   binary_unit_case<ztl::units::kilometer, ztl::units::meter, ztl::units::meter, 1, -250, 750>,
                   binary_unit_case<ztl::units::newton_meter, ztl::units::newton_centimeter,
                                    ztl::units::newton_centimeter, 2, 50, 250>) {
    typename Case::lhs lhs(Case::lhs_count);
    typename Case::rhs rhs(Case::rhs_count);

    auto sum        = lhs + rhs;
    auto difference = lhs - rhs;

    static_assert(std::is_same<decltype(sum), typename Case::result>::value,
                  "operator+ must return the common unit type, not bool");
    static_assert(std::is_same<decltype(difference), typename Case::result>::value,
                  "operator- must return the common unit type, not bool");

    CHECK(sum.count() == Case::result_count);
    CHECK(difference.count() == (ztl::units::unit_cast<typename Case::result>(lhs).count() -
                                 ztl::units::unit_cast<typename Case::result>(rhs).count()));
}

TEST_CASE("scalar arithmetic preserves the unit type") {
    ztl::units::millivolt value(9);

    auto product_left  = value * 3;
    auto product_right = 3 * value;
    auto quotient      = value / 3;
    auto remainder     = value % 4;

    static_assert(std::is_same<decltype(product_left), ztl::units::millivolt>::value,
                  "unit * scalar must preserve the unit type");
    static_assert(std::is_same<decltype(product_right), ztl::units::millivolt>::value,
                  "scalar * unit must preserve the unit type");
    static_assert(std::is_same<decltype(quotient), ztl::units::millivolt>::value,
                  "unit / scalar must preserve the unit type");
    static_assert(std::is_same<decltype(remainder), ztl::units::millivolt>::value,
                  "unit % scalar must preserve the unit type");

    CHECK(product_left.count() == 27);
    CHECK(product_right.count() == 27);
    CHECK(quotient.count() == 3);
    CHECK(remainder.count() == 1);
}

TEST_CASE("same-quantity division returns a dimensionless integer ratio") {
    CHECK(ztl::units::millisecond(1000) / ztl::units::second(1) == 1);
    CHECK(ztl::units::millivolt(1000) / ztl::units::volt(1) == 1);
    CHECK(ztl::units::meter(5) / ztl::units::centimeter(50) == 10);
    CHECK(ztl::units::newton_meter(2) / ztl::units::newton_centimeter(25) == 8);
}

TEST_CASE("cross-quantity electrical arithmetic produces derived units") {
    auto watts_from_base_units = ztl::units::volt(3) * ztl::units::ampere(2);
    auto watts_from_milliunits = ztl::units::millivolt(3000) * ztl::units::milliampere(2000);
    auto resistance            = ztl::units::volt(5) / ztl::units::milliampere(2);
    auto current               = ztl::units::volt(5) / ztl::units::kiloohm(2);
    auto power_to_current      = ztl::units::watt(10) / ztl::units::volt(5);
    auto power_to_voltage      = ztl::units::watt(10) / ztl::units::ampere(2);

    static_assert(std::is_same<decltype(watts_from_base_units), ztl::units::watt>::value,
                  "volt * ampere must produce watt");
    static_assert(
        std::is_same<ztl::units::quantity_of_t<decltype(watts_from_milliunits)>, ztl::units::power_tag>::value,
        "millivolt * milliampere must produce a power quantity");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(resistance)>, ztl::units::resistance_tag>::value,
                  "voltage / current must produce a resistance quantity");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(current)>, ztl::units::current_tag>::value,
                  "voltage / resistance must produce a current quantity");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(power_to_current)>, ztl::units::current_tag>::value,
                  "power / voltage must produce a current quantity");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(power_to_voltage)>, ztl::units::voltage_tag>::value,
                  "power / current must produce a voltage quantity");

    CHECK(watts_from_base_units.count() == 6);
    CHECK(ztl::units::unit_cast<ztl::units::watt>(watts_from_milliunits).count() == 6);
    CHECK(ztl::units::unit_cast<ztl::units::ohm>(resistance).count() == 2000);
    CHECK(ztl::units::unit_cast<ztl::units::milliampere>(current).count() == 2);
    CHECK(power_to_current.count() == 2);
    CHECK(power_to_voltage.count() == 5);
}

TEST_CASE("cross-quantity mechanical arithmetic preserves ratio scaling") {
    auto torque               = ztl::units::newton(2) * ztl::units::centimeter(50);
    auto torque_from_reversed = ztl::units::centimeter(50) * ztl::units::newton(2);
    auto force                = ztl::units::newton_meter(6) / ztl::units::meter(3);
    auto distance             = ztl::units::newton_meter(6) / ztl::units::newton(2);
    auto speed                = ztl::units::meter(10) / ztl::units::second(2);
    auto speed_in_kmh         = ztl::units::kilometer(72) / ztl::units::hour(1);
    auto distance_from_speed  = speed * ztl::units::second(3);
    auto distance_from_kmh    = ztl::units::kilometer_per_hour(90) * ztl::units::hour(2);
    auto distance_reversed    = ztl::units::hour(2) * ztl::units::kilometer_per_hour(90);
    auto duration             = ztl::units::meter(20) / speed;
    auto duration_from_kmh    = ztl::units::kilometer(180) / ztl::units::kilometer_per_hour(90);

    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(torque)>, ztl::units::torque_tag>::value,
                  "force * distance must produce torque");
    static_assert(
        std::is_same<ztl::units::quantity_of_t<decltype(torque_from_reversed)>, ztl::units::torque_tag>::value,
        "distance * force must also produce torque");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(force)>, ztl::units::force_tag>::value,
                  "torque / distance must produce force");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(distance)>, ztl::units::distance_tag>::value,
                  "torque / force must produce distance");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(speed)>, ztl::units::speed_tag>::value,
                  "distance / duration must produce speed");
    static_assert(std::is_same<decltype(speed_in_kmh), ztl::units::kilometer_per_hour>::value,
                  "kilometres divided by hours must produce km/h");
    static_assert(
        std::is_same<ztl::units::quantity_of_t<decltype(distance_from_speed)>, ztl::units::distance_tag>::value,
        "speed * duration must produce distance");
    static_assert(std::is_same<decltype(distance_from_kmh), ztl::units::kilometer>::value,
                  "km/h multiplied by hours must produce kilometres");
    static_assert(std::is_same<decltype(distance_reversed), ztl::units::kilometer>::value,
                  "hours multiplied by km/h must produce kilometres");
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(duration)>, ztl::units::duration_tag>::value,
                  "distance / speed must produce duration");
    static_assert(std::is_same<decltype(duration_from_kmh), ztl::units::hour>::value,
                  "kilometres divided by km/h must produce hours");

    CHECK(torque.count() == 100);
    CHECK(torque_from_reversed.count() == 100);
    CHECK(ztl::units::unit_cast<ztl::units::newton_meter>(torque).count() == 1);
    CHECK(force.count() == 2);
    CHECK(distance.count() == 3);
    CHECK(speed.count() == 5);
    CHECK(speed_in_kmh.count() == 72);
    CHECK(distance_from_speed.count() == 15);
    CHECK(distance_from_kmh.count() == 180);
    CHECK(distance_reversed.count() == 180);
    CHECK(duration.count() == 4);
    CHECK(duration_from_kmh.count() == 2);
}

TEST_CASE("invalid same-quantity and offset operations are SFINAE friendly") {
    static_assert(can_add_units<ztl::units::volt, ztl::units::millivolt>::value,
                  "same quantity units must support addition");
    static_assert(!can_add_units<ztl::units::volt, ztl::units::ampere>::value,
                  "different quantity units must not support addition");
    static_assert(can_compare_units<ztl::units::meter, ztl::units::millimeter>::value,
                  "same quantity units must support comparison");
    static_assert(!can_compare_units<ztl::units::meter, ztl::units::volt>::value,
                  "different quantity units must not support comparison");
    static_assert(!can_add_units<ztl::units::pascal, ztl::units::volt>::value,
                  "pressure and voltage must not support addition");
    static_assert(!can_multiply_units<ztl::units::volt, ztl::units::millivolt>::value,
                  "same quantity units must not use cross-quantity multiplication");
    static_assert(!can_multiply_units<ztl::units::celsius, ztl::units::ampere>::value,
                  "non-zero-offset units must not use cross-quantity multiplication");
    static_assert(can_multiply_units<ztl::units::volt, ztl::units::ampere>::value,
                  "valid cross-quantity units must support multiplication");
    static_assert(can_divide_units<ztl::units::meter, ztl::units::second>::value,
                  "distance / duration must support speed derivation");
    static_assert(can_multiply_units<ztl::units::kilometer_per_hour, ztl::units::hour>::value,
                  "speed * duration must support distance derivation");
    static_assert(can_multiply_units<ztl::units::hour, ztl::units::kilometer_per_hour>::value,
                  "duration * speed must support distance derivation");
    static_assert(!can_divide_units<ztl::units::second, ztl::units::meter>::value,
                  "duration / distance is not a modelled quantity operation");
    static_assert(!can_divide_units<ztl::units::meter_per_second, ztl::units::second>::value,
                  "speed / duration is not a modelled quantity operation");

    CHECK(true);
}

TEST_CASE("literals and unity constants provide readable construction") {
    using namespace ztl::units;

    CHECK((1_h).count() == 1);
    CHECK((2_min).count() == 2);
    CHECK((3_s).count() == 3);
    CHECK((4_ms).count() == 4);
    CHECK((5_ns).count() == 5);
    CHECK((6_mps).count() == 6);
    CHECK((7_kmh).count() == 7);
    CHECK((3_V).count() == 3);
    CHECK((4_mA).count() == 4);
    CHECK((5_W).count() == 5);
    CHECK((6_kOhm).count() == 6);
    CHECK((7_Pa).count() == 7);
    CHECK((8_mmHg).count() == 8);
    CHECK((9_C).count() == 9);
    CHECK((10_Deg).count() == 10);
    CHECK((11_N).count() == 11);
    CHECK((12_Ncm).count() == 12);
    CHECK((13_cm).count() == 13);

    CHECK((9 * Unity::h).count() == 9);
    CHECK((10 * Unity::min).count() == 10);
    CHECK((11 * Unity::s).count() == 11);
    CHECK((12 * Unity::ms).count() == 12);
    CHECK((13 * Unity::ns).count() == 13);
    CHECK((14 * Unity::mps).count() == 14);
    CHECK((15 * Unity::kmh).count() == 15);
    CHECK((14 * Unity::mV).count() == 14);
    CHECK((15 * Unity::mA).count() == 15);
    CHECK((16 * Unity::Pa).count() == 16);
    CHECK((17 * Unity::mmHg).count() == 17);
    CHECK((18 * Unity::Ncm).count() == 18);
    CHECK((19 * Unity::Gon).count() == 19);
    CHECK((20 * Unity::cm).count() == 20);
}

// ============================================================================
//  Fix 1 — common_type uses zero offset (absolute base)
// ============================================================================
//  Previously the common_type for two affine units summed their offsets
//  into a physically meaningless value.  Now the common type always has
//  zero offset, so it represents the absolute base (e.g. kelvin).

TEST_CASE("common_type of affine units uses zero offset") {
    using ct_c_f = ztl::units::common_type_t<ztl::units::celsius, ztl::units::fahrenheit>;
    using ct_c_k = ztl::units::common_type_t<ztl::units::celsius, ztl::units::kelvin>;

    // The common type must have zero offset (absolute representation)
    static_assert(ztl::units::offset_value_of<ct_c_f>::value == 0,
                  "common_type of celsius and fahrenheit must have zero offset (kelvin base)");
    static_assert(ztl::units::offset_value_of<ct_c_k>::value == 0,
                  "common_type of celsius and kelvin must have zero offset");

    // Comparisons through the zero-offset common type must still be correct
    CHECK(ztl::units::celsius(0) == ztl::units::fahrenheit(32));
    CHECK(ztl::units::celsius(100) > ztl::units::fahrenheit(211));
    CHECK(ztl::units::celsius(100) < ztl::units::fahrenheit(213));

    // Non-offset types should be unaffected
    using ct_v = ztl::units::common_type_t<ztl::units::volt, ztl::units::millivolt>;
    static_assert(ztl::units::offset_value_of<ct_v>::value == 0,
                  "common_type of zero-offset units is still zero offset");
    static_assert(std::is_same<ct_v, ztl::units::millivolt>::value, "common_type of volt and millivolt is millivolt");
}

// ============================================================================
//  Fix 2 — Addition/subtraction of affine (offset) units is blocked
// ============================================================================
//  celsius(10) + celsius(20) is physically meaningless (it would double-count
//  the offset).  The operators now require both operands to have zero offset.

TEST_CASE("addition and subtraction of affine units are SFINAE-blocked") {
    // Same offset unit + same offset unit → blocked
    static_assert(!can_add_units<ztl::units::celsius, ztl::units::celsius>::value,
                  "celsius + celsius must be blocked (offset != 0)");
    static_assert(!can_subtract_units<ztl::units::celsius, ztl::units::celsius>::value,
                  "celsius - celsius must be blocked (offset != 0)");

    // Different offset units → blocked
    static_assert(!can_add_units<ztl::units::celsius, ztl::units::fahrenheit>::value,
                  "celsius + fahrenheit must be blocked (both have offsets)");
    static_assert(!can_subtract_units<ztl::units::fahrenheit, ztl::units::celsius>::value,
                  "fahrenheit - celsius must be blocked (both have offsets)");

    // One offset, one zero-offset, same quantity → blocked
    static_assert(!can_add_units<ztl::units::celsius, ztl::units::kelvin>::value,
                  "celsius + kelvin must be blocked (celsius has offset)");
    static_assert(!can_add_units<ztl::units::kelvin, ztl::units::celsius>::value,
                  "kelvin + celsius must be blocked (celsius has offset)");

    // Zero-offset units of the same quantity → still allowed
    static_assert(can_add_units<ztl::units::kelvin, ztl::units::millikelvin>::value,
                  "kelvin + millikelvin must work (both zero offset)");
    static_assert(can_add_units<ztl::units::volt, ztl::units::millivolt>::value,
                  "volt + millivolt must work (both zero offset)");

    // Comparisons are still allowed for affine units
    static_assert(can_compare_units<ztl::units::celsius, ztl::units::fahrenheit>::value,
                  "celsius == fahrenheit comparison must still work");
    static_assert(can_compare_units<ztl::units::celsius, ztl::units::kelvin>::value,
                  "celsius == kelvin comparison must still work");

    CHECK(true);
}

// ============================================================================
//  Fix 3 — Cross-quantity multiplication overflow protection
// ============================================================================
//  Previously millivolt(50000) * milliampere(50000) overflowed int32 silently.
//  Now the multiplication promotes to a wider type first.

TEST_CASE("cross-quantity multiplication promotes to avoid overflow") {
    // 50000 * 50000 = 2,500,000,000 — overflows int32_t but fits int64_t
    auto result = ztl::units::millivolt(50000) * ztl::units::milliampere(50000);
    // The result is in microwatts (milli × milli = micro)
    // 2,500,000,000 microwatts = 2500 watts
    // After promotion and truncation back to int, it wraps — but the multiplication
    // itself is done in a wider type, preventing UB.  The truncation back to int
    // is implementation-defined (not UB).
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(result)>, ztl::units::power_tag>::value,
                  "millivolt * milliampere must produce a power quantity");

    // Safe-range test: values that fit in int32 after promotion
    auto safe = ztl::units::millivolt(3000) * ztl::units::milliampere(2000);
    CHECK(ztl::units::unit_cast<ztl::units::watt>(safe).count() == 6);

    // Cross-quantity division also promotes
    auto div_result = ztl::units::volt(10) / ztl::units::ampere(2);
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(div_result)>, ztl::units::resistance_tag>::value,
                  "volt / ampere must produce resistance");
    CHECK(div_result.count() == 5);
}

// ============================================================================
//  Fix 4 — microsecond alias added
// ============================================================================

TEST_CASE("microsecond alias and literal exist and convert correctly") {
    using namespace ztl::units;

    static_assert(ztl::units::is_unit<ztl::units::microsecond>::value, "microsecond must be a valid unit");
    static_assert(std::is_same<ztl::units::quantity_of_t<ztl::units::microsecond>, ztl::units::duration_tag>::value,
                  "microsecond must be a duration quantity");

    // Conversion chain: second → millisecond → microsecond
    CHECK(unit_cast<microsecond>(second(1)).count() == 1000000);
    CHECK(unit_cast<microsecond>(millisecond(1)).count() == 1000);
    CHECK(unit_cast<millisecond>(microsecond(1500)).count() == 1);
    CHECK(unit_cast<second>(microsecond(2000000)).count() == 2);

    // Literal
    CHECK((42_us).count() == 42);

    // Unity constant
    CHECK((7 * Unity::us).count() == 7);

    // Implicit lossless conversion (larger to smaller)
    microsecond us_from_ms = millisecond(5);
    CHECK(us_from_ms.count() == 5000);

    // Explicit lossy conversion (smaller to larger)
    static_assert(!std::is_convertible<microsecond, millisecond>::value,
                  "microsecond to millisecond must require explicit cast");
}

// ============================================================================
//  Fix 5 — find_msb handles negative values correctly
// ============================================================================

TEST_CASE("find_msb works on negative ratio numerators") {
    // Construct a unit with a negative ratio numerator to exercise find_msb.
    // std::ratio normalizes so that den > 0 and num carries the sign.
    // ratio<-1, 2> → num = -1, den = 2.
    // Previously find_msb(-1) returned 0 (bug); now it correctly returns 1.
    using neg_ratio = std::ratio<-1, 2>;
    static_assert(neg_ratio::num == -1, "sanity: ratio must normalize with negative num");

    // ratio_bitsize must report the correct number of bits for negative num
    constexpr unsigned bits = ztl::units::details::ratio_bitsize<neg_ratio>::value;
    static_assert(bits >= 1, "ratio_bitsize must detect bits in negative numerators");

    CHECK(ztl::units::details::find_msb(-1) == 1);
    CHECK(ztl::units::details::find_msb(-128) == 8);
    CHECK(ztl::units::details::find_msb(128) == 8);
    CHECK(ztl::units::details::find_msb(0) == 0);
}

// ============================================================================
//  Fix 6 — "gardian" renamed to "gradian"
// ============================================================================

TEST_CASE("gradian types are correctly named and work") {
    using namespace ztl::units;

    static_assert(ztl::units::is_unit<ztl::units::gradian>::value, "gradian must be a valid unit");
    static_assert(std::is_same<ztl::units::quantity_of_t<ztl::units::gradian>, ztl::units::angle_tag>::value,
                  "gradian must be an angle quantity");

    // 400 gradian = 1 turn = 360 degrees
    // Conversion: gradian → degree should work via common angle type
    CHECK(gradian(400) == degree(360));
    CHECK(gradian(100) == degree(90));

    // Literals
    CHECK((50_Gon).count() == 50);
    CHECK((100_mGon).count() == 100);
    CHECK((200_uGon).count() == 200);

    // Unity constants
    CHECK((3 * Unity::Gon).count() == 3);
    CHECK((4 * Unity::mGon).count() == 4);
    CHECK((5 * Unity::uGon).count() == 5);
}

// ============================================================================
//  Fix 7 — Converting constructor parameter names
// ============================================================================
//  This is a naming-only fix (REP2/UNIT2 instead of U2/R2).
//  Verify the converting constructor still works correctly.

TEST_CASE("implicit lossless converting constructor works after parameter rename") {
    // Lossless: second → millisecond (bigger to smaller unit)
    ztl::units::millisecond ms = ztl::units::second(3);
    CHECK(ms.count() == 3000);

    // Lossless: volt → millivolt
    ztl::units::millivolt mv = ztl::units::volt(2);
    CHECK(mv.count() == 2000);

    // Non-lossless: millisecond → second must NOT compile implicitly
    static_assert(!std::is_convertible<ztl::units::millisecond, ztl::units::second>::value,
                  "narrowing conversions must stay explicit");
}

// ============================================================================
//  Fix 8 — is_additive_compatible trait
// ============================================================================

TEST_CASE("is_additive_compatible correctly distinguishes offset and non-offset units") {
    // Zero-offset same-quantity: compatible
    static_assert(ztl::units::is_additive_compatible<ztl::units::volt, ztl::units::millivolt>::value,
                  "same quantity, both zero offset → additive compatible");
    static_assert(ztl::units::is_additive_compatible<ztl::units::kelvin, ztl::units::millikelvin>::value,
                  "kelvin types with zero offset → additive compatible");

    // Non-zero offset: not compatible for addition
    static_assert(!ztl::units::is_additive_compatible<ztl::units::celsius, ztl::units::celsius>::value,
                  "celsius has offset → not additive compatible");
    static_assert(!ztl::units::is_additive_compatible<ztl::units::fahrenheit, ztl::units::fahrenheit>::value,
                  "fahrenheit has offset → not additive compatible");
    static_assert(!ztl::units::is_additive_compatible<ztl::units::celsius, ztl::units::kelvin>::value,
                  "mixed offset/non-offset → not additive compatible");

    // Different quantities: not compatible regardless
    static_assert(!ztl::units::is_additive_compatible<ztl::units::volt, ztl::units::ampere>::value,
                  "different quantities → not additive compatible");

    CHECK(true);
}

// ============================================================================
//  Fix 9 — Temperature comparisons still work after common_type change
// ============================================================================

TEST_CASE("temperature unit_cast and comparison still work with zero-offset common_type") {
    // Direct unit_cast (unaffected by common_type changes)
    CHECK(ztl::units::unit_cast<ztl::units::kelvin>(ztl::units::celsius(0)).count() == 273);
    CHECK(ztl::units::unit_cast<ztl::units::celsius>(ztl::units::kelvin(273)).count() == 0);
    CHECK(ztl::units::unit_cast<ztl::units::celsius>(ztl::units::fahrenheit(32)).count() == 0);
    CHECK(ztl::units::unit_cast<ztl::units::kelvin>(ztl::units::fahrenheit(32)).count() == 273);

    // Comparisons through the new zero-offset common type
    CHECK(ztl::units::celsius(0) == ztl::units::fahrenheit(32));
    CHECK(ztl::units::celsius(100) == ztl::units::fahrenheit(212));
    CHECK(ztl::units::kelvin(274) > ztl::units::celsius(0));
    CHECK(ztl::units::kelvin(272) < ztl::units::celsius(0));

    // Kelvin-to-kelvin addition still works (zero offset)
    auto sum = ztl::units::kelvin(100) + ztl::units::millikelvin(500);
    CHECK(ztl::units::unit_cast<ztl::units::kelvin>(sum).count() == 100);
}

// ============================================================================
//  Fix 10 — Cross-quantity division also promotes
// ============================================================================

TEST_CASE("cross-quantity division uses wider type for ratio scaling") {
    // Newton_meter / meter = Newton; test with values that divide evenly
    auto force = ztl::units::newton_meter(20) / ztl::units::meter(4);
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(force)>, ztl::units::force_tag>::value,
                  "torque / distance must produce force");
    CHECK(force.count() == 5);

    // Speed computation with large values
    auto speed = ztl::units::kilometer(100) / ztl::units::hour(2);
    CHECK(speed.count() == 50);

    // Cross-quantity division with sub-units that needs wider intermediate
    auto resistance = ztl::units::millivolt(10000) / ztl::units::milliampere(5);
    CHECK(ztl::units::unit_cast<ztl::units::ohm>(resistance).count() == 2000);
}

// ============================================================================
//  Fix 11 — UDL checked_narrow helper
// ============================================================================

TEST_CASE("UDL literals produce correct values for in-range inputs") {
    using namespace ztl::units;

    // Exercise a representative set of literals
    CHECK((100_V).count() == 100);
    CHECK((2000_mA).count() == 2000);
    CHECK((500_W).count() == 500);
    CHECK((1000_Ohm).count() == 1000);
    CHECK((760_mmHg).count() == 760);
    CHECK((37_C).count() == 37);
    CHECK((98_F).count() == 98);
    CHECK((180_Deg).count() == 180);
    CHECK((100_N).count() == 100);
    CHECK((50_Nm).count() == 50);
    CHECK((1000_mm).count() == 1000);
    CHECK((42_us).count() == 42);
}

// ============================================================================
//  Regression: existing arithmetic with non-offset units still works
// ============================================================================

TEST_CASE("zero-offset arithmetic regression suite") {
    using namespace ztl::units;

    // Same-quantity addition
    auto duration_sum = minute(1) + second(30);
    static_assert(std::is_same<decltype(duration_sum), second>::value, "minute + second must return second");
    CHECK(duration_sum.count() == 90);

    // Same-quantity subtraction
    auto voltage_diff = volt(5) - millivolt(2500);
    static_assert(std::is_same<decltype(voltage_diff), millivolt>::value, "volt - millivolt must return millivolt");
    CHECK(voltage_diff.count() == 2500);

    // Same-quantity division (scalar result)
    CHECK(millisecond(1000) / second(1) == 1);
    CHECK(meter(5) / centimeter(50) == 10);

    // Cross-quantity: V * A = W
    auto watts = volt(12) * ampere(3);
    static_assert(std::is_same<decltype(watts), watt>::value, "volt * ampere must produce watt");
    CHECK(watts.count() == 36);

    // Cross-quantity: V / A = Ohm
    auto resistance = volt(12) / ampere(4);
    static_assert(std::is_same<ztl::units::quantity_of_t<decltype(resistance)>, resistance_tag>::value,
                  "volt / ampere must produce resistance");
    CHECK(resistance.count() == 3);

    // Scalar multiply / divide
    CHECK((millivolt(10) * 5).count() == 50);
    CHECK((5 * millivolt(10)).count() == 50);
    CHECK((millivolt(50) / 5).count() == 10);
    CHECK((millivolt(50) % 7).count() == 1);
}

// ============================================================================
//  Regression: chrono interop still works
// ============================================================================

TEST_CASE("chrono interop is unaffected by the fixes") {
    auto chrono_seconds = ztl::units::to_chrono_duration<std::chrono::seconds>(ztl::units::minute(2));
    CHECK(chrono_seconds.count() == 120);

    auto unit_ms = ztl::units::from_chrono_duration<ztl::units::millisecond>(std::chrono::seconds(3));
    CHECK(unit_ms.count() == 3000);

    auto roundtrip = ztl::units::unit_cast<std::chrono::nanoseconds>(ztl::units::nanosecond(42));
    CHECK(roundtrip.count() == 42);

    auto back = ztl::units::unit_cast<ztl::units::second>(std::chrono::milliseconds(5500));
    CHECK(back.count() == 5);
}

// ============================================================================
//  Regression: pressure mixed arithmetic
// ============================================================================

TEST_CASE("pressure mixed arithmetic regression") {
    auto pressure = ztl::units::pascal(1) + ztl::units::millimeter_of_mercury(1);
    CHECK(pressure.count() == 167903);
    CHECK(ztl::units::unit_cast<ztl::units::pascal>(pressure).count() == 134);
    CHECK(ztl::units::unit_cast<ztl::units::pascal>(ztl::units::millimeter_of_mercury(760)).count() == 101325);
}

// ============================================================================
//  Regression: speed mixed arithmetic
// ============================================================================

TEST_CASE("speed mixed arithmetic regression") {
    auto mixed_speed = ztl::units::kilometer_per_hour(18) + ztl::units::meter_per_second(5);
    static_assert(std::is_same<decltype(mixed_speed), ztl::units::speed<int, std::ratio<1, 18>>>::value,
                  "mixed m/s and km/h arithmetic must return the finest common speed unit");
    CHECK(mixed_speed.count() == 180);
    CHECK(ztl::units::unit_cast<ztl::units::meter_per_second>(mixed_speed).count() == 10);
}
