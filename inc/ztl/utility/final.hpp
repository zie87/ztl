// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_UTILITY_FINAL_HPP
#define ZTL_UTILITY_FINAL_HPP

#include "ztl/macros.hpp"

#include <type_traits>
#include <utility>

namespace ztl {

template <class F>
class final_action {
public:
    explicit final_action(const F& action) noexcept(std::is_nothrow_copy_constructible<F>::value) : m_action{action} {}
    explicit final_action(F&& action) noexcept(std::is_nothrow_move_constructible<F>::value)
        : m_action{std::move(action)} {}

    final_action(final_action&& other) noexcept(std::is_nothrow_move_constructible<F>::value)
        : m_action(std::move(other.m_action)), m_invoke(other.m_invoke) {
        other.m_invoke = false;
    }

    ~final_action() noexcept {
        if (m_invoke) {
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

template <class F>
ZTL_NODISCARD inline auto
finally(F&& f) noexcept(noexcept(final_action<typename std::decay<F>::type>(std::forward<F>(f))))
    -> final_action<typename std::decay<F>::type> {
    return final_action<typename std::decay<F>::type>(std::forward<F>(f));
}

} // namespace ztl

#endif // ZTL_UTILITY_FINAL_HPP
