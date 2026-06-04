#include <doctest/doctest.h>

#include "ztl/cobs.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace
{

template<std::size_t ACTUAL_N, std::size_t EXPECTED_N>
void require_prefix_eq(const std::array<ztl::byte, ACTUAL_N>&   actual,
                       std::size_t                              actual_size,
                       const std::array<ztl::byte, EXPECTED_N>& expected)
{
    REQUIRE(actual_size == expected.size());

    for (std::size_t i = 0u; i < expected.size(); ++i)
    {
        CHECK(actual[i] == expected[i]);
    }
}

template<std::size_t N>
void require_same_payload(const std::array<ztl::byte, N>& actual,
                          std::size_t                     actual_size,
                          const std::array<ztl::byte, N>& expected)
{
    REQUIRE(actual_size == expected.size());

    for (std::size_t i = 0u; i < expected.size(); ++i)
    {
        CHECK(actual[i] == expected[i]);
    }
}

template<std::size_t INPUT_N, std::size_t ENCODED_N>
void require_cobs_vector(const std::array<ztl::byte, INPUT_N>&   input,
                         const std::array<ztl::byte, ENCODED_N>& expected_encoded)
{
    std::array<ztl::byte, ztl::cobs::encoded_max_size(INPUT_N)> encoded = {};
    const ztl::cobs::result                                     encode_result
        = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                            ztl::as_writable_bytes(ztl::make_span(encoded)));

    REQUIRE(encode_result.success);
    CHECK(static_cast<bool>(encode_result));
    require_prefix_eq(encoded, encode_result.size, expected_encoded);

    std::array<ztl::byte, INPUT_N> decoded = {};
    const ztl::cobs::result        decode_result
        = ztl::cobs::decode(ztl::as_bytes(ztl::make_span(expected_encoded)),
                            ztl::as_writable_bytes(ztl::make_span(decoded)));

    REQUIRE(decode_result.success);
    CHECK(static_cast<bool>(decode_result));
    require_same_payload(decoded, decode_result.size, input);
}

template<std::size_t N>
void require_round_trip(const std::array<ztl::byte, N>& input)
{
    std::array<ztl::byte, ztl::cobs::encoded_max_size(N)> encoded = {};
    std::array<ztl::byte, N>                              decoded = {};

    const ztl::cobs::result encode_result
        = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                            ztl::as_writable_bytes(ztl::make_span(encoded)));

    REQUIRE(encode_result.success);
    REQUIRE(encode_result.size <= encoded.size());

    for (std::size_t i = 0u; i < encode_result.size; ++i)
    {
        CHECK(encoded[i] != ztl::to_byte(0u));
    }

    const ztl::cobs::result decode_result
        = ztl::cobs::decode(ztl::as_bytes(ztl::make_span(encoded.data(), encode_result.size)),
                            ztl::as_writable_bytes(ztl::make_span(decoded)));

    REQUIRE(decode_result.success);
    require_same_payload(decoded, decode_result.size, input);
}

} // namespace

TEST_SUITE("ztl::cobs")
{
    TEST_CASE("encoded_max_size returns tight raw COBS upper bounds")
    {
        static_assert(ztl::cobs::encoded_max_size(0u) == 1u,
                      "empty payload encodes to one code byte");
        static_assert(ztl::cobs::encoded_max_size(1u) == 2u, "one byte payload");
        static_assert(ztl::cobs::encoded_max_size(253u) == 254u, "253 byte payload");
        static_assert(ztl::cobs::encoded_max_size(254u) == 255u, "exact 254 byte block");
        static_assert(ztl::cobs::encoded_max_size(255u) == 257u, "254 byte block plus one byte");
        static_assert(ztl::cobs::encoded_max_size(508u) == 510u, "two exact 254 byte blocks");
        static_assert(ztl::cobs::encoded_max_size(509u) == 512u,
                      "two exact 254 byte blocks plus one byte");

        CHECK(ztl::cobs::encoded_max_size(0u) == 1u);
        CHECK(ztl::cobs::encoded_max_size(254u) == 255u);
        CHECK(ztl::cobs::encoded_max_size(508u) == 510u);
    }

    TEST_CASE("decoded_max_size returns raw COBS decoded upper bounds")
    {
        static_assert(ztl::cobs::decoded_max_size(0u) == 0u,
                      "empty encoded input is invalid but has no payload capacity");
        static_assert(ztl::cobs::decoded_max_size(1u) == 0u,
                      "one code byte can decode to empty payload");
        static_assert(ztl::cobs::decoded_max_size(2u) == 1u,
                      "two encoded bytes can decode to one payload byte");
        static_assert(ztl::cobs::decoded_max_size(255u) == 254u, "full COBS block");

        CHECK(ztl::cobs::decoded_max_size(0u) == 0u);
        CHECK(ztl::cobs::decoded_max_size(1u) == 0u);
        CHECK(ztl::cobs::decoded_max_size(255u) == 254u);
    }

    TEST_CASE("known vectors encode and decode correctly")
    {
        require_cobs_vector(std::array<ztl::byte, 0u>{},
                            std::array<ztl::byte, 1u>{{ztl::to_byte(0x01u)}});

        require_cobs_vector(
            std::array<ztl::byte, 1u>{
                {ztl::to_byte(0x00u)}
        },
            std::array<ztl::byte, 2u>{{ztl::to_byte(0x01u), ztl::to_byte(0x01u)}});

        require_cobs_vector(
            std::array<ztl::byte, 1u>{
                {ztl::to_byte(0x11u)}
        },
            std::array<ztl::byte, 2u>{{ztl::to_byte(0x02u), ztl::to_byte(0x11u)}});

        require_cobs_vector(
            std::array<ztl::byte, 4u>{
                {ztl::to_byte(0x11u),
                 ztl::to_byte(0x22u),
                 ztl::to_byte(0x33u),
                 ztl::to_byte(0x44u)}
        },
            std::array<ztl::byte, 5u>{{ztl::to_byte(0x05u),
                                       ztl::to_byte(0x11u),
                                       ztl::to_byte(0x22u),
                                       ztl::to_byte(0x33u),
                                       ztl::to_byte(0x44u)}});

        require_cobs_vector(
            std::array<ztl::byte, 4u>{
                {ztl::to_byte(0x11u),
                 ztl::to_byte(0x22u),
                 ztl::to_byte(0x00u),
                 ztl::to_byte(0x33u)}
        },
            std::array<ztl::byte, 5u>{{ztl::to_byte(0x03u),
                                       ztl::to_byte(0x11u),
                                       ztl::to_byte(0x22u),
                                       ztl::to_byte(0x02u),
                                       ztl::to_byte(0x33u)}});

        require_cobs_vector(
            std::array<ztl::byte, 2u>{
                {ztl::to_byte(0x00u), ztl::to_byte(0x00u)}
        },
            std::array<ztl::byte, 3u>{
                {ztl::to_byte(0x01u), ztl::to_byte(0x01u), ztl::to_byte(0x01u)}});

        require_cobs_vector(
            std::array<ztl::byte, 4u>{
                {ztl::to_byte(0x11u),
                 ztl::to_byte(0x00u),
                 ztl::to_byte(0x00u),
                 ztl::to_byte(0x22u)}
        },
            std::array<ztl::byte, 5u>{{ztl::to_byte(0x02u),
                                       ztl::to_byte(0x11u),
                                       ztl::to_byte(0x01u),
                                       ztl::to_byte(0x02u),
                                       ztl::to_byte(0x22u)}});
    }

    TEST_CASE("decode accepts an empty decoded payload with an empty output buffer")
    {
        const std::array<ztl::byte, 1u> encoded = {{ztl::to_byte(0x01u)}};
        std::array<ztl::byte, 0u>       decoded = {};

        const ztl::cobs::result result
            = ztl::cobs::decode(ztl::as_bytes(ztl::make_span(encoded)),
                                ztl::as_writable_bytes(ztl::make_span(decoded)));

        REQUIRE(result.success);
        CHECK(result.size == 0u);
    }

    TEST_CASE("decode accepts output buffers sized to decoded_max_size rather than encoded size")
    {
        const std::array<ztl::byte, 2u> encoded = {
            {ztl::to_byte(0x02u), ztl::to_byte(0x11u)}
        };
        std::array<ztl::byte, ztl::cobs::decoded_max_size(2u)> decoded = {};

        const ztl::cobs::result result
            = ztl::cobs::decode(ztl::as_bytes(ztl::make_span(encoded)),
                                ztl::as_writable_bytes(ztl::make_span(decoded)));

        REQUIRE(result.success);
        CHECK(result.size == 1u);
        CHECK(decoded[0u] == ztl::to_byte(0x11u));
    }

    TEST_CASE("encode handles exactly one full 254 byte non-zero block")
    {
        std::array<ztl::byte, 254u> input = {};
        std::fill(input.begin(), input.end(), ztl::to_byte(0xA5u));

        std::array<ztl::byte, ztl::cobs::encoded_max_size(254u)> encoded = {};
        const ztl::cobs::result                                  result
            = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                                ztl::as_writable_bytes(ztl::make_span(encoded)));

        REQUIRE(result.success);
        CHECK(result.size == 255u);
        CHECK(encoded[0u] == ztl::to_byte(0xFFu));

        for (std::size_t i = 1u; i < result.size; ++i)
        {
            CHECK(encoded[i] == ztl::to_byte(0xA5u));
        }

        require_round_trip(input);
    }

    TEST_CASE("encode starts a new block after 254 non-zero bytes when more input remains")
    {
        std::array<ztl::byte, 255u> input = {};
        std::fill(input.begin(), input.end(), ztl::to_byte(0xA5u));
        input[254u] = ztl::to_byte(0x5Au);

        std::array<ztl::byte, ztl::cobs::encoded_max_size(255u)> encoded = {};
        const ztl::cobs::result                                  result
            = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                                ztl::as_writable_bytes(ztl::make_span(encoded)));

        REQUIRE(result.success);
        CHECK(result.size == 257u);
        CHECK(encoded[0u] == ztl::to_byte(0xFFu));
        CHECK(encoded[255u] == ztl::to_byte(0x02u));
        CHECK(encoded[256u] == ztl::to_byte(0x5Au));

        require_round_trip(input);
    }

    TEST_CASE("encode handles two exact 254 byte non-zero blocks")
    {
        std::array<ztl::byte, 508u> input = {};
        std::fill(input.begin(), input.end(), ztl::to_byte(0x7Eu));

        std::array<ztl::byte, ztl::cobs::encoded_max_size(508u)> encoded = {};
        const ztl::cobs::result                                  result
            = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                                ztl::as_writable_bytes(ztl::make_span(encoded)));

        REQUIRE(result.success);
        CHECK(result.size == 510u);
        CHECK(encoded[0u] == ztl::to_byte(0xFFu));
        CHECK(encoded[255u] == ztl::to_byte(0xFFu));

        require_round_trip(input);
    }

    TEST_CASE("round trips representative payload shapes")
    {
        // Empty payload round-trip is covered by the known-vector and empty-decode tests.
        // Keeping it out of this generic helper avoids a GCC -Warray-bounds false positive
        // when the encoded prefix length flows through a dynamic-extent span.
        require_round_trip(std::array<ztl::byte, 6u>{
            {ztl::to_byte(0x00u),
             ztl::to_byte(0x11u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x22u),
             ztl::to_byte(0x33u),
             ztl::to_byte(0x00u)}
        });
        require_round_trip(std::array<ztl::byte, 8u>{
            {ztl::to_byte(0x01u),
             ztl::to_byte(0x02u),
             ztl::to_byte(0x03u),
             ztl::to_byte(0x04u),
             ztl::to_byte(0x05u),
             ztl::to_byte(0x06u),
             ztl::to_byte(0x07u),
             ztl::to_byte(0x08u)}
        });
        require_round_trip(std::array<ztl::byte, 8u>{
            {ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u),
             ztl::to_byte(0x00u)}
        });
    }

    TEST_CASE("encode fails when output buffer is too small")
    {
        const std::array<ztl::byte, 0u> empty_input  = {};
        std::array<ztl::byte, 0u>       empty_output = {};

        const ztl::cobs::result empty_result = ztl::cobs::encode(
            ztl::as_bytes(ztl::make_span(empty_input)),
            ztl::as_writable_bytes(ztl::make_span(empty_output.data(), empty_output.size())));
        CHECK(!empty_result.success);
        CHECK(empty_result.size == 0u);

        const std::array<ztl::byte, 1u> input             = {{ztl::to_byte(0x11u)}};
        std::array<ztl::byte, 1u>       undersized_output = {};

        const ztl::cobs::result result
            = ztl::cobs::encode(ztl::as_bytes(ztl::make_span(input)),
                                ztl::as_writable_bytes(ztl::make_span(undersized_output.data(),
                                                                      undersized_output.size())));
        CHECK(!result.success);
        CHECK(result.size == 0u);
    }

    TEST_CASE("decode rejects malformed encoded input")
    {
        std::array<ztl::byte, 4u> output = {};

        const std::array<ztl::byte, 0u> empty = {};
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(empty)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);

        const std::array<ztl::byte, 1u> zero_code = {{ztl::to_byte(0x00u)}};
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(zero_code)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);

        const std::array<ztl::byte, 1u> code_larger_than_remaining = {{ztl::to_byte(0x02u)}};
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(code_larger_than_remaining)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);

        const std::array<ztl::byte, 2u> zero_inside_encoded_data = {
            {ztl::to_byte(0x02u), ztl::to_byte(0x00u)}
        };
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(zero_inside_encoded_data)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);

        const std::array<ztl::byte, 2u> delimiter_in_raw_encoded_input = {
            {ztl::to_byte(0x01u), ztl::to_byte(0x00u)}
        };
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(delimiter_in_raw_encoded_input)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);

        const std::array<ztl::byte, 3u> truncated_full_block = {
            {ztl::to_byte(0xFFu), ztl::to_byte(0x11u), ztl::to_byte(0x22u)}
        };
        CHECK(!ztl::cobs::decode(ztl::as_bytes(ztl::make_span(truncated_full_block)),
                                 ztl::as_writable_bytes(ztl::make_span(output)))
                   .success);
    }

    TEST_CASE("decode fails when output buffer is too small for actual payload")
    {
        const std::array<ztl::byte, 3u> encoded = {
            {ztl::to_byte(0x03u), ztl::to_byte(0x11u), ztl::to_byte(0x22u)}
        };
        std::array<ztl::byte, 1u> undersized_output = {};

        const ztl::cobs::result result
            = ztl::cobs::decode(ztl::as_bytes(ztl::make_span(encoded)),
                                ztl::as_writable_bytes(ztl::make_span(undersized_output.data(),
                                                                      undersized_output.size())));

        CHECK(!result.success);
        CHECK(result.size == 0u);
    }
}
