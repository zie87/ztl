#include <doctest/doctest.h>

#include "ztl/byte.hpp"
#include "ztl/type_traits.hpp"

#include <cstdint>
#include <type_traits>

namespace {

constexpr ztl::byte make_byte(unsigned value) noexcept {
    return static_cast<ztl::byte>(static_cast<std::uint8_t>(value));
}

// clang-format off
template<typename L, typename R, typename = void>
struct has_plus : std::false_type {};

template<typename L, typename R>
struct has_plus<L, R, ztl::void_t<decltype(std::declval<L>() + std::declval<R>())>> : std::true_type {};

template<typename L, typename R, typename = void>
struct has_minus : std::false_type {};

template<typename L, typename R>
struct has_minus<L, R, ztl::void_t<decltype(std::declval<L>() - std::declval<R>())>> : std::true_type {};

template<typename L, typename R, typename = void>
struct has_multiply : std::false_type {};

template<typename L, typename R>
struct has_multiply<L, R, ztl::void_t<decltype(std::declval<L>() * std::declval<R>())>> : std::true_type {};

template<typename T, typename = void>
struct has_logical_not : std::false_type {};

template<typename T>
struct has_logical_not<T, ztl::void_t<decltype(!std::declval<T>())>> : std::true_type {};

template<typename B, typename Shift, typename = void>
struct has_left_shift : std::false_type {};

template<typename B, typename Shift>
struct has_left_shift<B, Shift, ztl::void_t<decltype(std::declval<B>() << std::declval<Shift>())>>
: std::true_type {};

template<typename B, typename Shift, typename = void>
struct has_right_shift : std::false_type {};

template<typename B, typename Shift>
struct has_right_shift<B, Shift, ztl::void_t<decltype(std::declval<B>() >> std::declval<Shift>())>> : std::true_type {};
// clang-format on

} // namespace

TEST_CASE("ztl::byte exposes byte-like type traits") {
    CHECK(std::is_enum<ztl::byte>::value);
    CHECK(std::is_trivially_copyable<ztl::byte>::value);
    CHECK(std::is_standard_layout<ztl::byte>::value);
    CHECK(sizeof(ztl::byte) == 1u);

    CHECK_FALSE((std::is_convertible<ztl::byte, int>::value));
    CHECK_FALSE((std::is_convertible<int, ztl::byte>::value));
}

TEST_CASE("ztl::byte converts explicitly to integral values") {
    constexpr auto b = make_byte(0xa5u);

    static_assert(ztl::to_integer<std::uint8_t>(b) == 0xa5u, "to_integer must be constexpr");
    static_assert(ztl::to_integer<unsigned>(b) == 0xa5u, "to_integer must support integral targets");

    CHECK(ztl::to_integer<std::uint8_t>(b) == 0xa5u);
    CHECK(ztl::to_integer<unsigned>(b) == 0xa5u);
    CHECK(ztl::to_integer<int>(b) == 0xa5);
}

TEST_CASE("ztl::byte supports bitwise operators") {
    const auto lhs = make_byte(0xacu);
    const auto rhs = make_byte(0x53u);

    CHECK(ztl::to_integer<unsigned>(lhs | rhs) == 0xffu);
    CHECK(ztl::to_integer<unsigned>(lhs & rhs) == 0x00u);
    CHECK(ztl::to_integer<unsigned>(lhs ^ rhs) == 0xffu);
    CHECK(ztl::to_integer<unsigned>(~make_byte(0x0fu)) == 0xf0u);
}

TEST_CASE("ztl::byte supports compound bitwise assignment operators") {
    auto value = make_byte(0x0fu);
    value |= make_byte(0xf0u);
    CHECK(ztl::to_integer<unsigned>(value) == 0xffu);

    value &= make_byte(0x33u);
    CHECK(ztl::to_integer<unsigned>(value) == 0x33u);

    value ^= make_byte(0x0fu);
    CHECK(ztl::to_integer<unsigned>(value) == 0x3cu);
}

TEST_CASE("ztl::byte supports integral shifts") {
    CHECK(ztl::to_integer<unsigned>(make_byte(0x01u) << 3u) == 0x08u);
    CHECK(ztl::to_integer<unsigned>(make_byte(0x80u) >> 7u) == 0x01u);

    auto value = make_byte(0x03u);
    value <<= 2u;
    CHECK(ztl::to_integer<unsigned>(value) == 0x0cu);

    value >>= 1u;
    CHECK(ztl::to_integer<unsigned>(value) == 0x06u);
}

TEST_CASE("ztl::byte rejects arithmetic and non-integral shift expressions") {
    CHECK_FALSE((has_plus<ztl::byte, ztl::byte>::value));
    CHECK_FALSE((has_minus<ztl::byte, ztl::byte>::value));
    CHECK_FALSE((has_multiply<ztl::byte, ztl::byte>::value));
    CHECK_FALSE((has_logical_not<ztl::byte>::value));

    CHECK((has_left_shift<ztl::byte, unsigned>::value));
    CHECK((has_right_shift<ztl::byte, unsigned>::value));
    CHECK_FALSE((has_left_shift<ztl::byte, float>::value));
    CHECK_FALSE((has_right_shift<ztl::byte, float>::value));
}

TEST_CASE("ztl::byte bit helpers read and modify individual bits") {
    auto value = make_byte(0x00u);

    CHECK_FALSE(ztl::get_bit(value, 0u));
    CHECK_FALSE(ztl::get_bit(value, 7u));

    ztl::set_bit(value, 0u);
    CHECK(ztl::to_integer<unsigned>(value) == 0x01u);
    CHECK(ztl::get_bit(value, 0u));

    ztl::set_bit(value, 7u, true);
    CHECK(ztl::to_integer<unsigned>(value) == 0x81u);
    CHECK(ztl::get_bit(value, 7u));

    ztl::clear_bit(value, 0u);
    CHECK(ztl::to_integer<unsigned>(value) == 0x80u);
    CHECK_FALSE(ztl::get_bit(value, 0u));

    ztl::toggle_bit(value, 7u);
    CHECK(ztl::to_integer<unsigned>(value) == 0x00u);
    CHECK_FALSE(ztl::get_bit(value, 7u));

    ztl::set_bit(value, 3u, false);
    CHECK(ztl::to_integer<unsigned>(value) == 0x00u);
}

namespace {
struct source_value final {
    std::uint16_t value;
};

struct target_value final {
    std::uint16_t value;
};
} // namespace

TEST_CASE("ztl::bit_cast copies bits between same-sized trivially copyable objects") {
    const source_value source{0x1234u};
    const auto         target = ztl::bit_cast<target_value>(source);

    CHECK(target.value == 0x1234u);

    const auto raw = ztl::bit_cast<std::uint16_t>(source);
    CHECK(raw == 0x1234u);
}
