// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_HASH_HPP
#define ZTL_HASH_HPP

#include <climits>
#include <cstddef>
#include <cstdint>

namespace ztl {

namespace details {
static_assert(CHAR_BIT == 8, "FNV-1a implementation requires 8-bit char");

struct fnv1a32_cfg {
    static constexpr std::uint32_t offset_basis = 2166136261u;
    static constexpr std::uint32_t prime        = 16777619u;
};

inline constexpr std::uint32_t fnv1a32_step(std::uint32_t h, unsigned char byte) noexcept {
    return static_cast<std::uint32_t>((h ^ static_cast<std::uint32_t>(byte)) * fnv1a32_cfg::prime);
}
} // namespace details

inline constexpr std::uint32_t fnv1a32_cstr(const char*   s,
                                            std::uint32_t h = details::fnv1a32_cfg::offset_basis) noexcept {
    while (*s != '\0') {
        h = details::fnv1a32_step(h, static_cast<unsigned char>(*s));
        ++s;
    }

    return h;
}

template <std::size_t N>
constexpr std::uint32_t fnv1a32_literal(const char (&s)[N],
                                        std::uint32_t h = details::fnv1a32_cfg::offset_basis) noexcept {
    for (std::size_t i = 0U; i + 1U < N; ++i) {
        h = details::fnv1a32_step(h, static_cast<unsigned char>(s[i]));
    }

    return h;
}

inline std::uint32_t fnv1a32_bytes(const void* data, std::size_t size,
                                   std::uint32_t h = details::fnv1a32_cfg::offset_basis) noexcept {
    const unsigned char* p = static_cast<const unsigned char*>(data);

    for (std::size_t i = 0U; i < size; ++i) {
        h = details::fnv1a32_step(h, p[i]);
    }

    return h;
}

static_assert(fnv1a32_cstr("hello") == 0x4F9F2CABu, "FNV-1a mismatch");
static_assert(fnv1a32_literal("hello") == 0x4F9F2CABu, "FNV-1a mismatch");

} // namespace ztl

#endif // ZTL_HASH_HPP
