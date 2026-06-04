// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_SOURCELOCATION_HPP
#define ZTL_SOURCELOCATION_HPP

#include "ztl/macros.hpp"

namespace ztl
{
// NOTE: backport of C++20 std::source-location
class source_location
{
public:
    static inline constexpr auto current(const char* fna, unsigned lin, const char* fun) noexcept
        -> source_location;

    constexpr source_location(const char* fna, unsigned lin, const char* fun) noexcept;

    inline constexpr unsigned    line() const noexcept;
    inline constexpr const char* file_name() const noexcept;
    inline constexpr const char* function_name() const noexcept;

private:
    const char* m_file_name;
    unsigned    m_line_number;
    const char* m_function_name;
};

inline constexpr source_location
source_location::current(const char* fna, unsigned lin, const char* fun) noexcept
{
    return source_location(fna, lin, fun);
}

constexpr source_location::source_location(const char* fna, unsigned lin, const char* fun) noexcept
: m_file_name(fna), m_line_number(lin), m_function_name(fun)
{
}

inline constexpr unsigned    source_location::line() const noexcept { return m_line_number; }
inline constexpr const char* source_location::file_name() const noexcept { return m_file_name; }
inline constexpr const char* source_location::function_name() const noexcept
{
    return m_function_name;
}
} // namespace ztl

#define CURRENT_SOURCE_LOCATION                                                                    \
    ::ztl::source_location::current(ZTL_FILE_BASENAME, ZTL_LINE, ZTL_FUNC)

#endif // ZTL_SOURCELOCATION_HPP
