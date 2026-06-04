#ifndef ZTL_ASSERT_HPP
#define ZTL_ASSERT_HPP

#include "ztl/source_location.hpp"

#include <type_traits>
#include <utility>

namespace ztl
{
// NOTE: assertion implementation inspiered by https://www.foonathan.net/2016/09/assertions/
namespace details
{
template<unsigned LVL_V>
using level = std::integral_constant<unsigned, LVL_V>;
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
    static constexpr unsigned level = 1U;

    [[noreturn]] static void handle(const source_location& loc,
                                    const char*            expression,
                                    const char*            message = nullptr) noexcept
    {
        (void)loc;
        (void)expression;
        (void)message;
        trap_forever();
    }
};

namespace details
{
template<typename Handler, typename... Args>
constexpr bool assert_is_enabled(Handler, Args&&...) noexcept
{
    return Handler::level != 0U;
}

template<typename Handler, unsigned Level, typename... Args>
constexpr bool assert_is_enabled(Handler, level<Level>, Args&&...) noexcept
{
    static_assert(Level > 0U, "level of an assertion must not be 0");
    return Level <= Handler::level;
}

template<typename Handler, typename... Args>
[[noreturn]] inline void
assert_fail(const source_location& loc, const char* expression, Handler, Args&&... args) noexcept
{
    Handler::handle(loc, expression, std::forward<Args>(args)...);
    trap_forever();
}

template<typename Handler, unsigned Level, typename... Args>
[[noreturn]] inline void assert_fail(const source_location& loc,
                                     const char*            expression,
                                     Handler,
                                     level<Level>,
                                     Args&&... args) noexcept
{
    static_assert(Level > 0U, "level of an assertion must not be 0");
    Handler::handle(loc, expression, std::forward<Args>(args)...);
    trap_forever();
}
} // namespace details
} // namespace ztl

#define ZTL_ASSERT(cond, ...)                                                                      \
    do                                                                                             \
    {                                                                                              \
        if (::ztl::details::assert_is_enabled(__VA_ARGS__))                                        \
        {                                                                                          \
            if (!(cond))                                                                           \
            {                                                                                      \
                ::ztl::details::assert_fail(ZTL_CURRENT_SOURCE_LOCATION, #cond, __VA_ARGS__);      \
            }                                                                                      \
        }                                                                                          \
    } while (false)

#define ZTL_CONTRACT_CHECK(type, cond) ZTL_ASSERT((cond), ::ztl::default_assert_handler{}, type)
#define ZTL_EXPECTS(cond) ZTL_CONTRACT_CHECK("Precondition", cond)
#define ZTL_ENSURES(cond) ZTL_CONTRACT_CHECK("Postcondition", cond)

#endif
