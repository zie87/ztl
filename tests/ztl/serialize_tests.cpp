#include <doctest/doctest.h>

#include "ztl/serialize.hpp"
#include "ztl/span.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ztl_tests
{
namespace
{

template<typename T>
struct test_value;

template<>
struct test_value<std::uint8_t>
{
    static constexpr std::uint8_t value = 0xA0u;
};

template<>
struct test_value<std::uint16_t>
{
    static constexpr std::uint16_t value = 0xA053u;
};

template<>
struct test_value<std::uint32_t>
{
    static constexpr std::uint32_t value = 0x10A05322ul;
};

template<>
struct test_value<std::uint64_t>
{
    static constexpr std::uint64_t value = 0x0123456789ABCDEFull;
};

template<std::size_t N>
inline auto make_writable_byte_span(std::array<std::uint8_t, N>& buffer) noexcept
{
    ztl::span<std::uint8_t, N> view(buffer.data(), buffer.size());
    return ztl::as_writable_bytes(view);
}

template<std::size_t N>
inline auto make_const_byte_span(const std::array<std::uint8_t, N>& buffer) noexcept
{
    ztl::span<const std::uint8_t, N> view(buffer.data(), buffer.size());
    return ztl::as_bytes(view);
}

template<typename T>
inline auto to_unsigned(T value) noexcept -> typename std::make_unsigned<T>::type
{
    return static_cast<typename std::make_unsigned<T>::type>(value);
}

template<ztl::endian Endian, typename T>
inline auto expected_byte(T value, std::size_t index) noexcept -> std::uint8_t
{
    typedef typename std::make_unsigned<T>::type unsigned_type;

    const std::size_t byte_index = (Endian == ztl::endian::big) ? (sizeof(T) - 1U - index) : index;

    const unsigned_type shifted
        = static_cast<unsigned_type>(to_unsigned(value) >> (byte_index * 8U));

    return static_cast<std::uint8_t>(shifted & static_cast<unsigned_type>(0xFFU));
}

template<ztl::endian Endian, typename T, std::size_t N>
inline void fill_expected_bytes(T value, std::array<std::uint8_t, N>& buffer, std::size_t offset)
{
    for (std::size_t i = 0U; i < sizeof(T); ++i)
    {
        buffer[offset + i] = expected_byte<Endian>(value, i);
    }
}

template<ztl::endian Endian, typename T, std::size_t N>
inline void
check_serialized_bytes(T value, const std::array<std::uint8_t, N>& buffer, std::size_t offset)
{
    for (std::size_t i = 0U; i < sizeof(T); ++i)
    {
        CHECK(buffer[offset + i] == expected_byte<Endian>(value, i));
    }
}

template<std::size_t N>
inline void check_filled_with(const std::array<std::uint8_t, N>& buffer,
                              std::size_t                        first,
                              std::size_t                        last,
                              std::uint8_t                       expected)
{
    for (std::size_t i = first; i < last; ++i)
    {
        CHECK(buffer[i] == expected);
    }
}

} // namespace

TEST_CASE_TEMPLATE("ztl::serialize_to writes little-endian unsigned integers",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    constexpr std::uint8_t sentinel = 0xCCu;
    const T                value    = test_value<T>::value;

    std::array<std::uint8_t, 16U> buffer{};
    buffer.fill(sentinel);

    const std::size_t written
        = ztl::serialize_to<ztl::endian::little>(value, make_writable_byte_span(buffer));

    CHECK(written == sizeof(T));
    check_serialized_bytes<ztl::endian::little>(value, buffer, 0U);
    check_filled_with(buffer, sizeof(T), buffer.size(), sentinel);
}

TEST_CASE_TEMPLATE("ztl::serialize_to writes big-endian unsigned integers",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    constexpr std::uint8_t sentinel = 0xCCu;
    const T                value    = test_value<T>::value;

    std::array<std::uint8_t, 16U> buffer{};
    buffer.fill(sentinel);

    const std::size_t written
        = ztl::serialize_to<ztl::endian::big>(value, make_writable_byte_span(buffer));

    CHECK(written == sizeof(T));
    check_serialized_bytes<ztl::endian::big>(value, buffer, 0U);
    check_filled_with(buffer, sizeof(T), buffer.size(), sentinel);
}

TEST_CASE_TEMPLATE("ztl::serialize_to writes at the requested offset",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    constexpr std::uint8_t sentinel = 0x55u;
    constexpr std::size_t  offset   = 3U;
    const T                value    = test_value<T>::value;

    std::array<std::uint8_t, 16U> buffer{};
    buffer.fill(sentinel);

    const std::size_t written
        = ztl::serialize_to<ztl::endian::big>(value,
                                              make_writable_byte_span(buffer),
                                              static_cast<std::ptrdiff_t>(offset));

    CHECK(written == sizeof(T));
    check_filled_with(buffer, 0U, offset, sentinel);
    check_serialized_bytes<ztl::endian::big>(value, buffer, offset);
    check_filled_with(buffer, offset + sizeof(T), buffer.size(), sentinel);
}

TEST_CASE_TEMPLATE("ztl::deserialize_from reads little-endian unsigned integers",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    const T value = test_value<T>::value;

    std::array<std::uint8_t, sizeof(T)> buffer{};
    fill_expected_bytes<ztl::endian::little>(value, buffer, 0U);

    T                 result{};
    const std::size_t read
        = ztl::deserialize_from<ztl::endian::little>(result, make_const_byte_span(buffer));

    CHECK(read == sizeof(T));
    CHECK(result == value);
}

TEST_CASE_TEMPLATE("ztl::deserialize_from reads big-endian unsigned integers",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    const T value = test_value<T>::value;

    std::array<std::uint8_t, sizeof(T)> buffer{};
    fill_expected_bytes<ztl::endian::big>(value, buffer, 0U);

    T                 result{};
    const std::size_t read
        = ztl::deserialize_from<ztl::endian::big>(result, make_const_byte_span(buffer));

    CHECK(read == sizeof(T));
    CHECK(result == value);
}

TEST_CASE_TEMPLATE("ztl::deserialize_from reads from the requested offset",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    constexpr std::uint8_t sentinel = 0xAAu;
    constexpr std::size_t  offset   = 4U;
    const T                value    = test_value<T>::value;

    std::array<std::uint8_t, 16U> buffer{};
    buffer.fill(sentinel);
    fill_expected_bytes<ztl::endian::little>(value, buffer, offset);

    T                 result{};
    const std::size_t read
        = ztl::deserialize_from<ztl::endian::little>(result,
                                                     make_const_byte_span(buffer),
                                                     static_cast<std::ptrdiff_t>(offset));

    CHECK(read == sizeof(T));
    CHECK(result == value);
}

TEST_CASE_TEMPLATE("ztl::serialize_to and ztl::deserialize_from round-trip big-endian values",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    const T value = test_value<T>::value;

    std::array<std::uint8_t, sizeof(T)> buffer{};

    const std::size_t written
        = ztl::serialize_to<ztl::endian::big>(value, make_writable_byte_span(buffer));

    T                 result{};
    const std::size_t read
        = ztl::deserialize_from<ztl::endian::big>(result, make_const_byte_span(buffer));

    CHECK(written == sizeof(T));
    CHECK(read == sizeof(T));
    CHECK(result == value);
}

TEST_CASE_TEMPLATE("ztl::serialize_to and ztl::deserialize_from default to little-endian",
                   T,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t)
{
    const T value = test_value<T>::value;

    std::array<std::uint8_t, sizeof(T)> buffer{};

    const std::size_t written = ztl::serialize_to(value, make_writable_byte_span(buffer));
    check_serialized_bytes<ztl::endian::little>(value, buffer, 0U);

    T                 result{};
    const std::size_t read = ztl::deserialize_from(result, make_const_byte_span(buffer));

    CHECK(written == sizeof(T));
    CHECK(read == sizeof(T));
    CHECK(result == value);
}

} // namespace ztl_tests
