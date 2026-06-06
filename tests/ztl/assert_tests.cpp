// SPDX-License-Identifier: BSL-1.0

#include "ztl/assert.hpp"

#include <doctest/doctest.h>

#include <cstdlib>
#include <cstring>
#include <setjmp.h>

namespace {

struct recorded_assertion {
    bool        called     = false;
    const char* expression = nullptr;
    const char* message    = nullptr;
    int         code       = 0;
};

jmp_buf            assertion_jump;
bool               assertion_jump_active = false;
recorded_assertion recorded;

void reset_recorded_assertion() noexcept {
    assertion_jump_active = false;
    recorded              = recorded_assertion{};
}

[[noreturn]] void leave_assertion_handler() noexcept {
    if (assertion_jump_active) {
        longjmp(assertion_jump, 1);
    }

    std::abort();
}

[[noreturn]] void record_assertion(const ztl::source_location&, const char* expression,
                                   const char* message = nullptr) noexcept {
    recorded.called     = true;
    recorded.expression = expression;
    recorded.message    = message;

    leave_assertion_handler();
}

struct disabled_handler {
    static constexpr unsigned level = 0U;

    [[noreturn]] static void handle(const ztl::source_location& loc, const char* expression,
                                    const char* message = nullptr) noexcept {
        record_assertion(loc, expression, message);
    }
};

struct level_one_handler {
    static constexpr unsigned level = 1U;

    [[noreturn]] static void handle(const ztl::source_location& loc, const char* expression,
                                    const char* message = nullptr) noexcept {
        record_assertion(loc, expression, message);
    }
};

struct level_two_handler {
    static constexpr unsigned level = 2U;

    [[noreturn]] static void handle(const ztl::source_location& loc, const char* expression,
                                    const char* message = nullptr) noexcept {
        record_assertion(loc, expression, message);
    }
};

struct payload_handler {
    static constexpr unsigned level = 1U;

    [[noreturn]] static void handle(const ztl::source_location&, const char* expression, int code,
                                    const char* message) noexcept {
        recorded.called     = true;
        recorded.expression = expression;
        recorded.code       = code;
        recorded.message    = message;

        leave_assertion_handler();
    }
};

static_assert(!ztl::details::assert_is_enabled(disabled_handler{}), "level 0 disables unlevelled asserts");
static_assert(ztl::details::assert_is_enabled(level_one_handler{}), "non-zero level enables unlevelled asserts");
static_assert(ztl::details::assert_is_enabled(level_one_handler{}, ztl::details::level<1U>{}),
              "handler level 1 enables assert level 1");
static_assert(!ztl::details::assert_is_enabled(level_one_handler{}, ztl::details::level<2U>{}),
              "handler level 1 disables assert level 2");
static_assert(ztl::details::assert_is_enabled(level_two_handler{}, ztl::details::level<2U>{}),
              "handler level 2 enables assert level 2");

} // namespace

TEST_CASE("assert_is_enabled reports the handler and assertion level combination") {
    CHECK_FALSE(ztl::details::assert_is_enabled(disabled_handler{}));
    CHECK(ztl::details::assert_is_enabled(level_one_handler{}));

    CHECK(ztl::details::assert_is_enabled(level_one_handler{}, ztl::details::level<1U>{}));
    CHECK_FALSE(ztl::details::assert_is_enabled(level_one_handler{}, ztl::details::level<2U>{}));
    CHECK(ztl::details::assert_is_enabled(level_two_handler{}, ztl::details::level<2U>{}));
}

TEST_CASE("ZTL_ASSERT does not call the handler when the condition is true") {
    reset_recorded_assertion();

    bool condition = true;
    ZTL_ASSERT(condition, level_one_handler{}, "must not be reported");

    CHECK_FALSE(recorded.called);
    CHECK(recorded.expression == nullptr);
    CHECK(recorded.message == nullptr);
}

TEST_CASE("ZTL_ASSERT does not call the handler when the handler is disabled") {
    reset_recorded_assertion();

    bool condition = false;
    ZTL_ASSERT(condition, disabled_handler{}, "disabled assertion");

    CHECK_FALSE(recorded.called);
    CHECK(recorded.expression == nullptr);
    CHECK(recorded.message == nullptr);
}

TEST_CASE("ZTL_ASSERT does not call the handler when the assertion level is disabled") {
    reset_recorded_assertion();

    bool condition = false;
    ZTL_ASSERT(condition, level_one_handler{}, ztl::details::level<2U>{}, "disabled level");

    CHECK_FALSE(recorded.called);
    CHECK(recorded.expression == nullptr);
    CHECK(recorded.message == nullptr);
}

TEST_CASE("ZTL_ASSERT forwards failed unlevelled assertions to the handler") {
    reset_recorded_assertion();

    assertion_jump_active = true;
    if (setjmp(assertion_jump) == 0) {
        bool condition = false;
        ZTL_ASSERT(condition, level_one_handler{}, "failed assertion");
        assertion_jump_active = false;
        FAIL("ZTL_ASSERT did not call the assertion handler");
    } else {
        assertion_jump_active = false;

        REQUIRE(recorded.called);
        REQUIRE(recorded.expression != nullptr);
        REQUIRE(recorded.message != nullptr);
        CHECK(std::strcmp(recorded.expression, "condition") == 0);
        CHECK(std::strcmp(recorded.message, "failed assertion") == 0);
    }
}

TEST_CASE("ZTL_ASSERT forwards failed assertions without a message") {
    reset_recorded_assertion();

    assertion_jump_active = true;
    if (setjmp(assertion_jump) == 0) {
        bool condition = false;
        ZTL_ASSERT(condition, level_one_handler{});
        assertion_jump_active = false;
        FAIL("ZTL_ASSERT did not call the assertion handler");
    } else {
        assertion_jump_active = false;

        REQUIRE(recorded.called);
        REQUIRE(recorded.expression != nullptr);
        CHECK(recorded.message == nullptr);
        CHECK(std::strcmp(recorded.expression, "condition") == 0);
    }
}

TEST_CASE("ZTL_ASSERT forwards failed enabled level assertions to the handler") {
    reset_recorded_assertion();

    assertion_jump_active = true;
    if (setjmp(assertion_jump) == 0) {
        bool condition = false;
        ZTL_ASSERT(condition, level_two_handler{}, ztl::details::level<2U>{}, "level two assertion");
        assertion_jump_active = false;
        FAIL("ZTL_ASSERT did not call the assertion handler");
    } else {
        assertion_jump_active = false;

        REQUIRE(recorded.called);
        REQUIRE(recorded.expression != nullptr);
        REQUIRE(recorded.message != nullptr);
        CHECK(std::strcmp(recorded.expression, "condition") == 0);
        CHECK(std::strcmp(recorded.message, "level two assertion") == 0);
    }
}

TEST_CASE("ZTL_ASSERT forwards custom handler arguments") {
    reset_recorded_assertion();

    assertion_jump_active = true;
    if (setjmp(assertion_jump) == 0) {
        bool condition = false;
        ZTL_ASSERT(condition, payload_handler{}, 42, "payload assertion");
        assertion_jump_active = false;
        FAIL("ZTL_ASSERT did not call the assertion handler");
    } else {
        assertion_jump_active = false;

        REQUIRE(recorded.called);
        REQUIRE(recorded.expression != nullptr);
        REQUIRE(recorded.message != nullptr);
        CHECK(std::strcmp(recorded.expression, "condition") == 0);
        CHECK(recorded.code == 42);
        CHECK(std::strcmp(recorded.message, "payload assertion") == 0);
    }
}

TEST_CASE("contract macros compile and do not trap for satisfied contracts") {
    ZTL_EXPECTS(true);
    ZTL_ENSURES(1 == 1);

    CHECK(true);
}
