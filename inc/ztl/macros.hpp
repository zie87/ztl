// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_MACROS_HPP
#define ZTL_MACROS_HPP

namespace ztl
{
namespace details
{
constexpr const char* file_basename(const char* path) noexcept
{
    const char* last = path;
    for (const char* p = path; *p; ++p)
    {
        if (*p == '/' || *p == '\\')
        {
            last = p + 1;
        }
    }
    return last;
}
} // namespace details
} // namespace ztl

#define ZTL_STR_(x) #x
#define ZTL_STR(x) ZTL_STR_(x)

#define ZTL_FILE __FILE__
#define ZTL_LINE __LINE__
#define ZTL_FUNC __func__
#define ZTL_FILE_BASENAME (::ztl::details::file_basename(ZTL_FILE))

#endif /* ZTL_MACROS_HPP */
