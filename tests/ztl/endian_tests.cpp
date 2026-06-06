#include "ztl/endian.hpp"

#include <doctest/doctest.h>

#include <cstdint>
#include <type_traits>

namespace tests {
namespace {

template <typename>
struct test_values;

template <>
struct test_values<std::uint8_t> {
    static constexpr std::uint8_t little = 0xA0u;
    static constexpr std::uint8_t big    = little;
};

template <>
struct test_values<std::uint16_t> {
    static constexpr std::uint16_t little = 0xA053u;
    static constexpr std::uint16_t big    = 0x53A0u;
};

template <>
struct test_values<std::uint32_t> {
    static constexpr std::uint32_t little = 0x10A05322u;
    static constexpr std::uint32_t big    = 0x2253A010u;
};

template <>
struct test_values<std::uint64_t> {
    static constexpr std::uint64_t little = 0xff00aa5501020304ull;
    static constexpr std::uint64_t big    = 0x0403020155aa00ffull;
};

template <typename T>
constexpr auto native_value() noexcept -> T {
    return ztl::endian::native == ztl::endian::little ? test_values<T>::little : test_values<T>::big;
}

template <typename T>
constexpr auto little_value() noexcept -> T {
    return test_values<T>::little;
}

template <typename T>
constexpr auto big_value() noexcept -> T {
    return test_values<T>::big;
}

} // namespace

TEST_CASE("endian enum values [ztl][endian]") {
    CHECK(ztl::endian::little != ztl::endian::big);
    CHECK((ztl::endian::native == ztl::endian::little || ztl::endian::native == ztl::endian::big));
}

TEST_CASE_TEMPLATE("reverse_bytes [ztl][endian]", T, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t) {
    constexpr auto little = little_value<T>();
    constexpr auto big    = big_value<T>();

    SUBCASE("converts little-endian test value to big-endian test value") {
        constexpr auto result = ztl::reverse_bytes(little);
        CHECK(result == big);
    }

    SUBCASE("converts big-endian test value to little-endian test value") {
        constexpr auto result = ztl::reverse_bytes(big);
        CHECK(result == little);
    }

    SUBCASE("is reversible") {
        constexpr auto result = ztl::reverse_bytes(ztl::reverse_bytes(little));
        CHECK(result == little);
    }
}

TEST_CASE_TEMPLATE("byteswap [ztl][endian]", T, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t) {
    constexpr auto little = little_value<T>();
    constexpr auto big    = big_value<T>();

    SUBCASE("returns the same type") {
        CHECK((std::is_same<decltype(ztl::byteswap(T{})), T>::value));
    }

    SUBCASE("swaps bytes") {
        constexpr auto result = ztl::byteswap(little);
        CHECK(result == big);
    }

    SUBCASE("is reversible") {
        constexpr auto result = ztl::byteswap(ztl::byteswap(little));
        CHECK(result == little);
    }
}

TEST_CASE_TEMPLATE("endian conversion [ztl][endian]", T, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t) {
    constexpr auto native = native_value<T>();
    constexpr auto little = little_value<T>();
    constexpr auto big    = big_value<T>();

    SUBCASE("swap from little to big endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::little, ztl::endian::big>(little);
        CHECK(result == big);
    }

    SUBCASE("swap from big to little endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::big, ztl::endian::little>(big);
        CHECK(result == little);
    }

    SUBCASE("swap from native to little endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::native, ztl::endian::little>(native);
        CHECK(result == little);
    }

    SUBCASE("swap from native to big endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::native, ztl::endian::big>(native);
        CHECK(result == big);
    }

    SUBCASE("swap from big to native endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::big, ztl::endian::native>(big);
        CHECK(result == native);
    }

    SUBCASE("swap from little to native endian") {
        constexpr auto result = ztl::endian_swap<ztl::endian::little, ztl::endian::native>(little);
        CHECK(result == native);
    }

    SUBCASE("same-endian conversions do not change the value") {
        CHECK(ztl::endian_swap<ztl::endian::little, ztl::endian::little>(little) == little);
        CHECK(ztl::endian_swap<ztl::endian::big, ztl::endian::big>(big) == big);
        CHECK(ztl::endian_swap<ztl::endian::native, ztl::endian::native>(native) == native);
    }
}

} // namespace tests
