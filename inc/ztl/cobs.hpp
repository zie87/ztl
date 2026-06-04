// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_COBS_HPP
#define ZTL_COBS_HPP

#include "ztl/byte.hpp"
#include "ztl/span.hpp"
#include "ztl/macros.hpp"

#include <cstddef>
#include <cstdint>

namespace ztl
{

// Raw COBS (Consistent Overhead Byte Stuffing) encoder/decoder.
//
// Framing contract:
//   - encode() does NOT append the 0x00 frame delimiter; the caller owns it.
//   - decode() does NOT consume the 0x00 frame delimiter; strip it before calling.
//   - Both functions return {0, false} on any error; a true result with size==0
//     is returned only for a legitimately empty encode (input of length 0).
struct cobs
{
    struct result
    {
        std::size_t size;
        bool        success;

        inline constexpr explicit operator bool() const noexcept { return success; }
    };

    // Upper bound on encoded output length for an n-byte raw payload,
    // excluding the trailing 0x00 delimiter.
    // Every 254 input bytes require one extra code byte, plus one leading code byte.
    // The n==0 case produces a single 0x01 code byte (valid but unusual; callers
    // should decide whether to treat empty frames as an error at the framing layer).
    static inline constexpr auto encoded_max_size(std::size_t n) noexcept -> std::size_t
    {
        if (n == 0)
        {
            return 1;
        }
        return n + (n + 253) / 254;
    }

    // Conservative upper bound on decoded output length for an encoded_size-byte
    // COBS payload (excluding any 0x00 delimiter).
    // Tight bound: at least one code byte is consumed, so decoded <= encoded - 1.
    // The actual decoded size can be smaller when 0xFF blocks are present.
    static inline constexpr auto decoded_max_size(std::size_t encoded_size) noexcept -> std::size_t
    {
        return (encoded_size != 0u) ? (encoded_size - 1u) : 0u;
    }

    template<std::size_t EXT_IN_V, std::size_t EXT_OUT_V>
    ZTL_NODISCARD static inline constexpr auto
    encode(ztl::span<const ztl::byte, EXT_IN_V> input,
           ztl::span<ztl::byte, EXT_OUT_V>      output) noexcept -> result
    {
        // Static check when both extents are known at compile time (C++14 triple-OR form).
        static_assert((EXT_IN_V == ztl::dynamic_extent) || (EXT_OUT_V == ztl::dynamic_extent)
                          || (EXT_OUT_V >= encoded_max_size(EXT_IN_V)),
                      "COBS encode: output span is too small for the given input extent");

        const std::size_t in_size  = input.size();
        const std::size_t required = encoded_max_size(in_size);

        if (output.size() < required)
        {
            return {0u, false};
        }

        std::size_t   read_index  = 0u;
        std::size_t   write_index = 1u; // slot 0 is reserved for the first code byte
        std::size_t   code_index  = 0u; // index of the current block's code byte
        std::uint16_t code        = 1u; // value of the current code byte (block length + 1)

        while (read_index < in_size)
        {
            const ztl::byte value = input[read_index];
            ++read_index;

            if (value == to_byte(0u))
            {
                // Zero terminates the current block; write the code byte and start a new block.
                output[code_index] = to_byte(static_cast<std::uint8_t>(code));
                code_index         = write_index;
                ++write_index;
                code = 1u;
            }
            else
            {
                output[write_index] = value;
                ++write_index;
                ++code;

                // A block holds at most 254 non-zero payload bytes (code == 0xFF).
                // If the block is full and more input follows, close it and start a new block.
                // If this was the last byte, the normal post-loop write handles it correctly.
                if ((code == 0xFFu) && (read_index < in_size))
                {
                    output[code_index] = to_byte(0xFFu);
                    code_index         = write_index;
                    ++write_index;
                    code = 1u;
                }
            }
        }

        // Write the final code byte.
        output[code_index] = to_byte(static_cast<std::uint8_t>(code));
        return {write_index, true};
    }

    template<std::size_t EXT_IN_V, std::size_t EXT_OUT_V>
    ZTL_NODISCARD static inline constexpr auto
    decode(ztl::span<const ztl::byte, EXT_IN_V> input,
           ztl::span<ztl::byte, EXT_OUT_V>      output) noexcept -> result
    {
        // Static check when both extents are known at compile time (C++14 triple-OR form).
        static_assert((EXT_IN_V == ztl::dynamic_extent) || (EXT_OUT_V == ztl::dynamic_extent)
                          || (EXT_OUT_V >= decoded_max_size(EXT_IN_V)),
                      "COBS decode: output span is too small for the given input extent");

        const std::size_t in_size = input.size();

        if (in_size == 0u)
        {
            return {0u, false};
        }

        std::size_t read_index  = 0u;
        std::size_t write_index = 0u;

        while (read_index < in_size)
        {
            const std::size_t code = ztl::to_integer<std::size_t>(input[read_index]);
            ++read_index;

            // A zero code byte is never valid in a COBS-encoded stream.
            if (code == 0u)
            {
                return {0u, false};
            }

            // The block claims (code - 1) explicit data bytes; verify they are present.
            const std::size_t data_count = code - 1u;
            if (data_count > (in_size - read_index))
            {
                return {0u, false};
            }

            // Copy the (code - 1) explicit data bytes; they must all be non-zero.
            for (std::size_t i = 0u; i < data_count; ++i)
            {
                const ztl::byte value = input[read_index];
                ++read_index;

                if (value == to_byte(0u))
                {
                    return {0u, false}; // zero inside encoded payload is a framing error
                }

                if (write_index >= output.size())
                {
                    return {0u, false}; // decoded output does not fit
                }

                output[write_index] = value;
                ++write_index;
            }

            // A code value other than 0xFF signals that the original payload had a
            // zero byte after this block (unless this is the final block).
            if ((code != 0xFFu) && (read_index < in_size))
            {
                if (write_index >= output.size())
                {
                    return {0u, false};
                }

                output[write_index] = to_byte(0u);
                ++write_index;
            }
        }

        return {write_index, true};
    }
};

} // namespace ztl

#endif // ZTL_COBS_HPP
