#include "ztl/crc.hpp"

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace {
struct table_strategy {
    static constexpr auto value() noexcept -> ztl::crc_strategy { return ztl::crc_strategy::table; }
};

struct function_strategy {
    static constexpr auto value() noexcept -> ztl::crc_strategy { return ztl::crc_strategy::function; }
};

template <typename EXPECTED_T>
struct crc_test_vector {
    std::vector<std::uint8_t> buffer;
    EXPECTED_T                expected;
};
} // namespace

TEST_CASE_TEMPLATE("crc8 calculation", Strategy, table_strategy, function_strategy) {
    constexpr auto strategy = Strategy::value();

    const std::array<crc_test_vector<std::uint8_t>, 5> test_vectors{{
        {
            {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},
            0x45u,
        },
        {
            {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'},
            0x45u,
        },
        {
            {0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30},
            0x6eu,
        },
        {
            {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0'},
            0x6eu,
        },
        {
            {0x55, 0x0b, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01},
            0xf2u,
        },
    }};

    ztl::crc8<strategy> calculator{};

    std::size_t index = 0u;
    for (const auto& test_vector : test_vectors) {
        INFO("CRC8 test vector index: " << index);

        const auto result = calculator(std::cbegin(test_vector.buffer), std::cend(test_vector.buffer));
        CHECK(static_cast<unsigned>(result) == static_cast<unsigned>(test_vector.expected));

        ++index;
    }
}

TEST_CASE_TEMPLATE("crc16 calculation", Strategy, table_strategy, function_strategy) {
    constexpr auto strategy = Strategy::value();

    SUBCASE("CRC-16/ARC") {
        using crc_type = ztl::crc16<strategy>;

        crc_type calculator{};

        SUBCASE("check value as hex bytes") {
            constexpr std::array<std::uint8_t, 9> buffer{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
            constexpr std::uint16_t               expected = 0xbb3d;

            const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
            CHECK(result == expected);
        }

        SUBCASE("check value as characters") {
            constexpr std::array<std::uint8_t, 9> buffer{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
            constexpr std::uint16_t               expected = 0xbb3d;

            const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
            CHECK(result == expected);
        }

        SUBCASE("CRC over uint16_t data") {
            constexpr std::array<std::uint16_t, 4> buffer{0x0001, 0x0010, 0x0100, 0x1000};
            constexpr std::uint16_t                expected = 0x5093;

            const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
            CHECK(result == expected);
        }
    }

    SUBCASE("CRC-16/CCITT reflected test set") {
        using generator_type =
            typename ztl::crc_generator<strategy, std::uint16_t, 0x8408, ztl::crc_bit_order::reflected>::type;
        using crc_type = ztl::crc_base<std::uint16_t, generator_type, 0xffffu, 0xffffu>;

        const std::array<crc_test_vector<std::uint16_t>, 6> test_vectors{{
            {
                {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},
                0x906e,
            },
            {
                {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},
                0x3c16,
            },
            {
                {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'},
                0x3c16,
            },
            {
                {0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30},
                0x3eb1,
            },
            {
                {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0'},
                0x3eb1,
            },
            {
                {0x55, 0x0b, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01},
                0x0a80,
            },
        }};

        crc_type calculator{};

        std::size_t index = 0u;
        for (const auto& test_vector : test_vectors) {
            INFO("CRC16/CCITT reflected test vector index: " << index);

            const auto result = calculator(std::cbegin(test_vector.buffer), std::cend(test_vector.buffer));
            CHECK(result == test_vector.expected);

            ++index;
        }
    }

    SUBCASE("CRC-16/CCITT reflected") {
        using generator_type =
            typename ztl::crc_generator<strategy, std::uint16_t, 0x8408, ztl::crc_bit_order::reflected>::type;
        using crc_type = ztl::crc_base<std::uint16_t, generator_type, 0xffffu, 0xffffu>;
        crc_type calculator{};

        SUBCASE("CRC over uint16_t data") {
            constexpr std::array<std::uint16_t, 4> buffer{0x0001, 0x0010, 0x0100, 0x1000};
            constexpr std::uint16_t                expected = 0x0a21;

            const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
            CHECK(result == expected);
        }

        SUBCASE("real frame payload") {
            constexpr std::array<std::uint8_t, 156> buffer{
                0x55, 0x5a, 0x88, 0x97, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x91, 0x43, 0x6f, 0x6d, 0x62, 0x69,
                0x6e, 0x65, 0x20, 0x6f, 0x6e, 0x65, 0x20, 0x6d, 0x61, 0x63, 0x68, 0x69, 0x6e, 0x65, 0x20, 0x77,
                0x69, 0x74, 0x68, 0x20, 0x6f, 0x6e, 0x65, 0x20, 0x70, 0x65, 0x64, 0x61, 0x6c, 0x20, 0x28, 0x6f,
                0x6e, 0x6c, 0x79, 0x20, 0x6e, 0x65, 0x63, 0x65, 0x73, 0x73, 0x61, 0x72, 0x79, 0x20, 0x66, 0x6f,
                0x72, 0x20, 0x77, 0x69, 0x72, 0x65, 0x6c, 0x65, 0x73, 0x73, 0x20, 0x70, 0x65, 0x64, 0x61, 0x6c,
                0x29, 0x2e, 0x0a, 0x41, 0x64, 0x6a, 0x75, 0x73, 0x74, 0x20, 0x68, 0x6f, 0x72, 0x69, 0x7a, 0x6f,
                0x6e, 0x74, 0x61, 0x6c, 0x2f, 0x76, 0x65, 0x72, 0x74, 0x69, 0x63, 0x61, 0x6c, 0x20, 0x70, 0x65,
                0x64, 0x61, 0x6c, 0x20, 0x73, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x6d,
                0x65, 0x63, 0x68, 0x61, 0x6e, 0x69, 0x63, 0x61, 0x6c, 0x20, 0x70, 0x65, 0x64, 0x61, 0x6c, 0x20,
                0x73, 0x6e, 0x61, 0x70, 0x20, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x73, 0x2e};
            constexpr std::uint16_t expected = 0xe405;

            const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
            CHECK(result == expected);
        }
    }
}

TEST_CASE_TEMPLATE("crc32 calculation", Strategy, table_strategy, function_strategy) {
    constexpr auto strategy = Strategy::value();

    using crc_type = ztl::crc32<strategy>;
    crc_type calculator{};

    SUBCASE("CRC-32/ISO test set") {
        const std::array<crc_test_vector<std::uint32_t>, 5> test_vectors{{
            {
                {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},
                0xcbf43926u,
            },
            {
                {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},
                0xa684c7c6u,
            },
            {
                {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'},
                0xa684c7c6u,
            },
            {
                {0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30},
                0x83ddb0b5u,
            },
            {
                {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0'},
                0x83ddb0b5u,
            },
        }};

        std::size_t index = 0u;
        for (const auto& test_vector : test_vectors) {
            INFO("CRC32/ISO test vector index: " << index);

            const auto result = calculator(std::cbegin(test_vector.buffer), std::cend(test_vector.buffer));
            CHECK(result == test_vector.expected);

            ++index;
        }
    }

    SUBCASE("CRC-32/ISO simulator ping command") {
        constexpr std::array<std::uint8_t, 8> buffer{0x55, 0x0b, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01};
        constexpr std::uint32_t               expected = 0x4b865640u;

        const auto result = calculator(std::cbegin(buffer), std::cend(buffer));
        CHECK(result == expected);
    }

    SUBCASE("CRC32 can be updated byte by byte") {
        constexpr std::array<std::uint8_t, 8> buffer{0x55, 0x0b, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01};

        std::uint32_t crc_value = crc_type::initial_value();
        for (const auto value : buffer) {
            calculator.calc(&value, &value + 1, crc_value);
        }

        constexpr std::uint32_t expected = 0x4b865640u;
        const auto              result   = calculator.finalize(crc_value);
        CHECK(result == expected);
        CHECK(result == calculator(std::cbegin(buffer), std::cend(buffer)));
    }
}
