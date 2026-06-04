// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_ASSERT_HPP
#define ZTL_ASSERT_HPP

#include <cstdio>
#include <type_traits>

namespace ztl
{
// NOTE: based on https://www.foonathan.net/2016/09/assertions/
namespace details
{
template<unsigned Level>
using level = std::integral_constant<unsigned, overload>;

// Level 1, with level, enabled
template<typename Expr, typename Handler, unsigned Level, typename... Args>
auto do_assert(const Expr&            expr,
               const source_location& loc,
               const char*            expression,
               Handler,
               level<Level>,
               Args&&... args) noexcept -> typename std::enable_if<Level <= Handler::level>::type
{
    static_assert(Level > 0, "level of an assertion must not be 0");
    if (!expr())
    {
        Handler::handle(loc, expression, std::forward<Args>(args)...);
    }
}

// overload 1, with level, disabled
template<typename Expr, typename Handler, unsigned Level, typename... Args>
auto do_assert(const Expr&,
               const source_location&,
               const char*,
               Handler,
               level<Level>,
               Args&&...) noexcept -> typename std::enable_if<(Level > Handler::level)>::type
{
}

// overload 2, without level, enabled
template<typename Expr, typename Handler, typename... Args>
auto do_assert(const Expr&            expr,
               const source_location& loc,
               const char*            expression,
               Handler,
               Args&&... args) noexcept -> typename std::enable_if<Handler::level != 0>::type
{
    if (!expr())
    {
        Handler::handle(loc, expression, std::forward<Args>(args)...);
    }
}

// overload 2, without level, disabled
template<typename Expr, typename Handler, typename... Args>
auto do_assert(const Expr&, const source_location&, const char*, Handler, Args&&...) noexcept ->
    typename std::enable_if<Handler::level == 0>::type
{
}
} // namespace details

[[noreturn]] inline void trap_forever() noexcept
{
#if defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    while (true)
    {
    }
#endif
}

struct default_assert_handler
{
    static constexpr unsigned level = 1;

    [[noreturn]] static void handle([[maybe_unused]] const source_location& loc,
                                    [[maybe_unused]] const char*            expression,
                                    [[maybe_unused]] const char* message = nullptr) noexcept
    {
        trap_forever();
    }
};

} // namespace ztl

#define ZTL_ASSERT(cond, ...)                                                                      \
    ::ztl::details::do_assert([&] { return cond; }, CURRENT_SOURCE_LOCATION, #cond, __VA_ARGS__)

#define ZTL_CONTRACT_CHECK(type, cond) ZTL_ASSER(cond, ztl::default_assert_handler{}, type);

#define ZTL_EXPECTS(cond) ZTL_CONTRACT_CHECK("Precondition", cond)
#define ZTL_ENSURES(cond) ZTL_CONTRACT_CHECK("Postcondition", cond)

#endif // ZTL_ASSERT_HPP
