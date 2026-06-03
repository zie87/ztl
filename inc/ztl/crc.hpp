// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_CRC_HPP
#define ZTL_CRC_HPP

#include "ztl/narrow.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace ztl
{

enum class crc_bit_order
{
    msb_first,
    reflected,
};

enum class crc_strategy
{
    table,
    function
};

namespace details
{
template<typename T>
[[nodiscard]] constexpr auto width() noexcept -> std::size_t
{
    return sizeof(T) * 8u;
}

template<typename T>
[[nodiscard]] constexpr auto top_bit() noexcept -> T
{
    return ::ztl::narrow_cast<T>(T{1u} << (width<T>() - 1u));
}

template<typename T>
[[nodiscard]] constexpr auto reflect_value(T value) noexcept -> T
{
    T result{};

    for (std::size_t bit = 0; bit < width<T>(); ++bit)
    {
        const auto source_bit = ::ztl::narrow_cast<T>(T{1u} << bit);
        if ((value & source_bit) != 0u)
        {
            const auto target_position = (width<T>() - 1u) - bit;
            const auto target_bit      = ::ztl::narrow_cast<T>(T{1u} << target_position);
            result                     = ::ztl::narrow_cast<T>(result | target_bit);
        }
    }

    return result;
}

template<typename T, crc_bit_order BIT_ORDER_V>
struct byte_update;

template<typename T>
struct byte_update<T, crc_bit_order::msb_first>
{
    using value_type = T;
    using table_type = std::array<value_type, 256>;

    [[nodiscard]] static constexpr auto table_entry(std::size_t dividend,
                                                    value_type  polynomial) noexcept -> value_type
    {
        constexpr auto shift     = width<value_type>() - 8u;
        auto           remainder = ::ztl::narrow_cast<value_type>(dividend << shift);

        for (std::uint8_t bit = 0; bit < 8u; ++bit)
        {
            if ((remainder & top_bit<value_type>()) != 0u)
            {
                remainder = ::ztl::narrow_cast<value_type>((remainder << 1u) ^ polynomial);
            }
            else
            {
                remainder = ::ztl::narrow_cast<value_type>(remainder << 1u);
            }
        }

        return remainder;
    }

    static constexpr void
    update_bitwise(value_type& crc, std::uint8_t byte, value_type polynomial) noexcept
    {
        constexpr auto shift = width<value_type>() - 8u;
        crc = ::ztl::narrow_cast<value_type>(crc ^ (static_cast<value_type>(byte) << shift));

        for (std::uint8_t bit = 0; bit < 8u; ++bit)
        {
            if ((crc & top_bit<value_type>()) != 0u)
            {
                crc = ::ztl::narrow_cast<value_type>((crc << 1u) ^ polynomial);
            }
            else
            {
                crc = ::ztl::narrow_cast<value_type>(crc << 1u);
            }
        }
    }

    static constexpr void
    update_table(value_type& crc, std::uint8_t byte, const table_type& table) noexcept
    {
        constexpr auto shift   = width<value_type>() - 8u;
        const auto     shifted = ::ztl::narrow_cast<value_type>(crc >> shift);
        const auto     entry   = static_cast<std::size_t>((shifted ^ byte) & 0x00ffu);
        crc                    = ::ztl::narrow_cast<value_type>((crc << 8u) ^ table[entry]);
    }
};

template<typename T>
struct byte_update<T, crc_bit_order::reflected>
{
    using value_type = T;
    using table_type = std::array<value_type, 256>;

    [[nodiscard]] static constexpr auto table_entry(std::size_t dividend,
                                                    value_type  polynomial) noexcept -> value_type
    {
        auto remainder = ::ztl::narrow_cast<value_type>(dividend);

        for (std::uint8_t bit = 0; bit < 8u; ++bit)
        {
            if ((remainder & 0x0001u) != 0u)
            {
                remainder = ::ztl::narrow_cast<value_type>((remainder >> 1u) ^ polynomial);
            }
            else
            {
                remainder = ::ztl::narrow_cast<value_type>(remainder >> 1u);
            }
        }

        return remainder;
    }

    static constexpr void
    update_bitwise(value_type& crc, std::uint8_t byte, value_type polynomial) noexcept
    {
        crc = ::ztl::narrow_cast<value_type>(crc ^ byte);

        for (std::uint8_t bit = 0; bit < 8u; ++bit)
        {
            if ((crc & 0x0001u) != 0u)
            {
                crc = ::ztl::narrow_cast<value_type>((crc >> 1u) ^ polynomial);
            }
            else
            {
                crc = ::ztl::narrow_cast<value_type>(crc >> 1u);
            }
        }
    }

    static constexpr void
    update_table(value_type& crc, std::uint8_t byte, const table_type& table) noexcept
    {
        const auto entry = static_cast<std::size_t>((crc ^ byte) & 0x00ffu);
        crc              = ::ztl::narrow_cast<value_type>((crc >> 8u) ^ table[entry]);
    }
};

template<typename T, T POLYNOME_V, crc_bit_order BIT_ORDER_V>
struct crc_table_generator_impl
{
    using value_type = T;
    using table_type = std::array<value_type, 256>;

    [[nodiscard]] constexpr auto operator()(std::size_t idx) const noexcept -> value_type
    {
        return M_TABLE[idx];
    }

    constexpr void update(value_type& crc, std::uint8_t byte) const noexcept
    {
        details::byte_update<value_type, BIT_ORDER_V>::update_table(crc, byte, M_TABLE);
    }

private:
    [[nodiscard]] static constexpr auto generate_one(std::size_t dividend) noexcept -> value_type
    {
        return details::byte_update<value_type, BIT_ORDER_V>::table_entry(dividend, POLYNOME_V);
    }

    template<std::size_t... INDICES>
    [[nodiscard]] static constexpr auto generate(std::index_sequence<INDICES...>) noexcept
        -> table_type
    {
        return table_type{{generate_one(INDICES)...}};
    }

    [[nodiscard]] static constexpr auto generate() noexcept -> table_type
    {
        return generate(std::make_index_sequence<256>{});
    }

    static constexpr table_type M_TABLE = generate();
};

#if __cplusplus < 201703L
template<typename T, T POLYNOME_V, crc_bit_order BIT_ORDER_V>
constexpr typename crc_table_generator_impl<T, POLYNOME_V, BIT_ORDER_V>::table_type
    crc_table_generator_impl<T, POLYNOME_V, BIT_ORDER_V>::M_TABLE;
#endif

template<typename T, T POLYNOME_V, crc_bit_order BIT_ORDER_V>
struct crc_function_generator_impl
{
    using value_type = T;

    constexpr void update(value_type& crc, std::uint8_t byte) const noexcept
    {
        details::byte_update<value_type, BIT_ORDER_V>::update_bitwise(crc, byte, POLYNOME_V);
    }
};

} // namespace details

template<crc_strategy STRATEGY_V, typename T, T POLYNOME_V, crc_bit_order ORDER_V>
struct crc_generator;

template<typename T, T POLYNOME_V, crc_bit_order ORDER_V>
struct crc_generator<crc_strategy::table, T, POLYNOME_V, ORDER_V>
{
    using type = details::crc_table_generator_impl<T, POLYNOME_V, ORDER_V>;
};

template<typename T, T POLYNOME_V, crc_bit_order ORDER_V>
struct crc_generator<crc_strategy::function, T, POLYNOME_V, ORDER_V>
{
    using type = details::crc_function_generator_impl<T, POLYNOME_V, ORDER_V>;
};

template<typename T, class GENERATOR_T, T INITIAL_VALUE_V = T{}, T FINAL_XOR_VALUE_V = T{}>
struct crc_base
{
    using value_type     = T;
    using generator_type = GENERATOR_T;

    constexpr crc_base() noexcept = default;
    constexpr explicit crc_base(generator_type generator) noexcept : m_generator(generator) {}

    [[nodiscard]] static constexpr auto initial_value() noexcept -> value_type
    {
        return INITIAL_VALUE_V;
    }
    [[nodiscard]] static constexpr auto final_xor_value() noexcept -> value_type
    {
        return FINAL_XOR_VALUE_V;
    }

    [[nodiscard]] static constexpr auto finalize(value_type crc) noexcept -> value_type
    {
        return ::ztl::narrow_cast<value_type>(crc ^ FINAL_XOR_VALUE_V);
    }

    template<typename INPUT_ITERATOR>
    constexpr auto operator()(INPUT_ITERATOR first, INPUT_ITERATOR last) const noexcept
        -> value_type
    {
        value_type crc{INITIAL_VALUE_V};
        calc(first, last, crc);
        return finalize(crc);
    }

    template<typename INPUT_ITERATOR>
    constexpr inline auto
    calc(INPUT_ITERATOR first, INPUT_ITERATOR last, value_type& crc) const noexcept ->
        typename std::enable_if<sizeof(typename std::iterator_traits<INPUT_ITERATOR>::value_type)
                                    != sizeof(std::uint8_t),
                                void>::type
    {
        constexpr std::size_t value_size
            = sizeof(typename std::iterator_traits<INPUT_ITERATOR>::value_type);
        for (; first != last; ++first)
        {
            const auto  val = *first;
            const auto* ptr = reinterpret_cast<const std::uint8_t*>(&val);
            calc(ptr, ptr + value_size, crc);
        }
    }

    template<typename INPUT_ITERATOR>
    constexpr inline auto
    calc(INPUT_ITERATOR first, INPUT_ITERATOR last, value_type& crc) const noexcept ->
        typename std::enable_if<sizeof(typename std::iterator_traits<INPUT_ITERATOR>::value_type)
                                    == sizeof(std::uint8_t),
                                void>::type
    {
        std::for_each(first,
                      last,
                      [this, &crc](auto val)
                      {
                          const auto v = ::ztl::narrow_cast<std::uint8_t>(val);
                          update_crc(crc, v);
                      });
    }

    inline void update_crc(value_type& crc, std::uint8_t val) const noexcept
    {
        m_generator.update(crc, val);
    }

private:
    generator_type m_generator{};
};

template<crc_strategy STRATEGY_V>
using crc8 = crc_base<
    std::uint8_t,
    typename crc_generator<STRATEGY_V, std::uint8_t, 0x07, crc_bit_order::msb_first>::type>;

template<crc_strategy STRATEGY_V>
using crc16 = crc_base<std::uint16_t,
                       typename crc_generator<STRATEGY_V,
                                              std::uint16_t,
                                              details::reflect_value<std::uint16_t>(0x8005),
                                              crc_bit_order::reflected>::type>;

template<crc_strategy STRATEGY_V>
using crc32 = crc_base<
    std::uint32_t,
    typename crc_generator<STRATEGY_V, std::uint32_t, 0xedb88320u, crc_bit_order::reflected>::type,
    0xffffffffu,
    0xffffffffu>;

} // namespace ztl

#endif // ZTL_CRC_HPP
