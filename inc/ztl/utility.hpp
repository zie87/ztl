// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UTILITY_HPP
#define ZTL_UTILITY_HPP

#include "ztl/macros.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace ztl
{

// NOTE: Equivalent to C++17 std::size(container)
template<typename CONTAINER_T>
inline constexpr auto size(const CONTAINER_T& container) noexcept(noexcept(container.size()))
    -> decltype(container.size())
{
    return container.size();
}

// NOTE: Equivalent to C++17 std::size(array)
template<typename T, std::size_t N>
inline constexpr auto size(const T (&)[N]) noexcept -> std::size_t
{
    return N;
}

// NOTE: Equivalent to C++20 std::ssize(container)
template<typename CONTAINER_T>
inline constexpr auto ssize(const CONTAINER_T& container) noexcept(noexcept(container.size())) ->
    typename std::common_type<std::ptrdiff_t,
                              typename std::make_signed<decltype(container.size())>::type>::type
{
    typedef
        typename std::common_type<std::ptrdiff_t,
                                  typename std::make_signed<decltype(container.size())>::type>::type
            result_type;

    return static_cast<result_type>(container.size());
}

// NOTE: Equivalent to C++20 std::ssize(array)
template<typename T, std::size_t N>
inline constexpr auto ssize(const T (&)[N]) noexcept -> std::ptrdiff_t
{
    return static_cast<std::ptrdiff_t>(N);
}

template<class F>
class final_action
{
public:
    explicit final_action(const F& action) noexcept(std::is_nothrow_copy_constructible<F>::value)
    : m_action{action}
    {
    }
    explicit final_action(F&& action) noexcept(std::is_nothrow_move_constructible<F>::value)
    : m_action{std::move(action)}
    {
    }

    final_action(final_action&& other) noexcept(std::is_nothrow_move_constructible<F>::value)
    : m_action(std::move(other.m_action)), m_invoke(other.m_invoke)
    {
        other.m_invoke = false;
    }

    ~final_action() noexcept
    {
        if (m_invoke)
        {
            m_action();
        }
    }

    final_action(const final_action&)   = delete;
    void operator=(const final_action&) = delete;
    void operator=(final_action&&)      = delete;

    void cancel() noexcept { m_invoke = false; }

private:
    F    m_action;
    bool m_invoke = true;
};

template<class F>
ZTL_NODISCARD inline auto
finally(F&& f) noexcept(noexcept(final_action<typename std::decay<F>::type>(std::forward<F>(f))))
    -> final_action<typename std::decay<F>::type>
{
    return final_action<typename std::decay<F>::type>(std::forward<F>(f));
}

} // namespace ztl

#endif // ZTL_UTILITY_HPP
