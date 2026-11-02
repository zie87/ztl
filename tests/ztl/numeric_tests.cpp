#include "ztl/numeric.hpp"

#include <doctest/doctest.h>

#include <cstdint>
#include <limits>
#include <type_traits>

TEST_CASE("gcd and lcm of ordinary values") {
    static_assert(ztl::gcd(12, 18) == 6, "gcd must be constexpr");
    static_assert(ztl::lcm(4, 6) == 12, "lcm must be constexpr");

    CHECK(ztl::gcd(12, 18) == 6);
    CHECK(ztl::gcd(-12, 18) == 6);
    CHECK(ztl::gcd(12, -18) == 6);
    CHECK(ztl::gcd(0, 7) == 7);
    CHECK(ztl::gcd(-7, 0) == 7);
    CHECK(ztl::gcd(0, 0) == 0);

    CHECK(ztl::lcm(4, 6) == 12);
    CHECK(ztl::lcm(-4, 6) == 12);
    CHECK(ztl::lcm(0, 6) == 0);
    CHECK(ztl::lcm(7, 0) == 0);
}

TEST_CASE("gcd and lcm return the common type of mixed arguments") {
    static_assert(std::is_same<decltype(ztl::gcd(4, 6L)), long>::value, "common type of int and long");
    static_assert(std::is_same<decltype(ztl::lcm(std::int16_t{4}, std::int16_t{6})), std::int16_t>::value,
                  "same-type arguments keep their type");

    CHECK(ztl::gcd(std::int64_t{1} << 40, 6) == 2);
    CHECK(ztl::lcm(std::int16_t{-4}, std::int16_t{6}) == 12);
    CHECK(ztl::gcd(4u, 6u) == 2u);
    CHECK(ztl::lcm(std::uint64_t{1} << 62, std::uint64_t{2}) == std::uint64_t{1} << 62);
}

TEST_CASE("gcd and lcm handle the most negative value") {
    constexpr int          int_min   = std::numeric_limits<int>::min();
    constexpr std::int64_t int64_min = std::numeric_limits<std::int64_t>::min();

    // |INT_MIN| is not representable, but the result is; this used to overflow
    static_assert(ztl::gcd(int_min, 6) == 2, "gcd(INT_MIN, 6) must be a constant expression");
    CHECK(ztl::gcd(int_min, 6) == 2);
    CHECK(ztl::gcd(6, int_min) == 2);
    CHECK(ztl::gcd(int64_min, int64_min / 2) == std::int64_t{1} << 62);
    CHECK(ztl::lcm(-(1 << 30), 2) == (1 << 30));
}

TEST_CASE("abs_val returns the magnitude") {
    static_assert(ztl::details::abs_val(-5) == 5, "abs_val must be constexpr");
    CHECK(ztl::details::abs_val(std::int64_t{-7}) == 7);
    CHECK(ztl::details::abs_val(7u) == 7u);
}
