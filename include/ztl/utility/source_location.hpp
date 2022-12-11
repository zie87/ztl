/*
 * Copyright © Tobias Zindl 2022
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 *
*/

#ifndef ZTL_UTILITY_SOURCE_LOCATION_HPP
#define ZTL_UTILITY_SOURCE_LOCATION_HPP

#define ZTL_BUILTIN_FILE __builtin_FILE()
#define ZTL_BUILTIN_FUNCTION __builtin_FUNCTION()
#define ZTL_BUILTIN_LINE __builtin_LINE()

namespace ztl {

struct source_location {
    static consteval auto current(unsigned int linenum = ZTL_BUILTIN_LINE, const char* const file = ZTL_BUILTIN_FILE,
                                  const char* const function = ZTL_BUILTIN_FUNCTION) noexcept -> source_location {
        source_location result{};
        result.m_line     = linenum;
        result.m_file     = file;
        result.m_function = function;
        return result;
    }

    constexpr source_location() noexcept = default;

    [[nodiscard]] constexpr auto line() const noexcept -> unsigned int { return m_line; }
    [[nodiscard]] constexpr auto file_name() const noexcept -> const char* { return m_file; }
    [[nodiscard]] constexpr auto function_name() const noexcept -> const char* { return m_function; }

private:
    unsigned int m_line    = 0;
    const char* m_file     = "";
    const char* m_function = "";
};

}  // namespace ztl

#endif  // ZTL_UTILITY_SOURCE_LOCATION_HPP
