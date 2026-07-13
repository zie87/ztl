#include "ztl/narrow.hpp"
#include "ztl/units.hpp"

#include <doctest/doctest.h>

#include <cstdint>
#include <limits>
#include <type_traits>

// ============================================================================
//  narrow_cast — smoke tests (unchecked, must compile and be constexpr)
// ============================================================================

TEST_CASE("narrow_cast performs unchecked static_cast") {
    constexpr int a = ztl::narrow_cast<int>(42ULL);
    CHECK(a == 42);

    constexpr int16_t b = ztl::narrow_cast<int16_t>(int32_t(1000));
    CHECK(b == 1000);

    // Intentional truncation — no trap, no error
    constexpr int8_t c = ztl::narrow_cast<int8_t>(int32_t(300));
    CHECK(c == static_cast<int8_t>(300));
}

// ============================================================================
//  to_underlying — smoke tests
// ============================================================================

TEST_CASE("to_underlying converts enum to its underlying integer") {
    enum class color : uint8_t { red = 1, green = 2, blue = 3 };
    constexpr auto v = ztl::to_underlying(color::blue);

    static_assert(std::is_same<decltype(v), const uint8_t>::value, "to_underlying must return the underlying type");
    CHECK(v == 3);
}

// ============================================================================
//  narrow_is_safe — exhaustive compile-time verification of the check logic
// ============================================================================

TEST_CASE("narrow_is_safe rejects value-truncating conversions") {
    // int32 → int8: value too large
    static_assert(!ztl::details::narrow_is_safe<int8_t>(int32_t(128)), "128 > INT8_MAX");
    static_assert(!ztl::details::narrow_is_safe<int8_t>(int32_t(-129)), "-129 < INT8_MIN");

    // uint32 → uint8: value too large
    static_assert(!ztl::details::narrow_is_safe<uint8_t>(256U), "256 > UINT8_MAX");

    // int64 → int32: value too large
    static_assert(!ztl::details::narrow_is_safe<int32_t>(int64_t(2147483648LL)), "> INT32_MAX");
    static_assert(!ztl::details::narrow_is_safe<int32_t>(int64_t(-2147483649LL)), "< INT32_MIN");

    // unsigned long long → int: value too large
    static_assert(!ztl::details::narrow_is_safe<int>(5000000000ULL), "5e9 > INT_MAX");

    CHECK(true);
}

TEST_CASE("narrow_is_safe rejects sign-changing conversions") {
    // Signed negative → unsigned: sign flip
    static_assert(!ztl::details::narrow_is_safe<unsigned>(-1), "-1 → unsigned wraps");
    static_assert(!ztl::details::narrow_is_safe<uint8_t>(int8_t(-1)), "-1 → uint8 wraps");
    static_assert(!ztl::details::narrow_is_safe<uint32_t>(int32_t(-100)), "-100 → uint32 wraps");

    // Unsigned → signed: value exceeds signed max
    static_assert(!ztl::details::narrow_is_safe<int>(static_cast<unsigned>(std::numeric_limits<int>::max()) + 1u),
                  "INT_MAX+1 as unsigned wraps to negative");

    CHECK(true);
}

TEST_CASE("narrow_is_safe accepts value-preserving conversions") {
    // Identity
    static_assert(ztl::details::narrow_is_safe<int>(42), "same type, same value");

    // Wider → narrower, value fits
    static_assert(ztl::details::narrow_is_safe<int>(42ULL), "42 fits in int");
    static_assert(ztl::details::narrow_is_safe<int8_t>(int32_t(127)), "INT8_MAX boundary");
    static_assert(ztl::details::narrow_is_safe<int8_t>(int32_t(-128)), "INT8_MIN boundary");
    static_assert(ztl::details::narrow_is_safe<uint8_t>(255U), "UINT8_MAX boundary");

    // Signed → unsigned, non-negative value that fits
    static_assert(ztl::details::narrow_is_safe<unsigned>(0), "0 is safe");
    static_assert(ztl::details::narrow_is_safe<unsigned>(42), "positive int fits");
    static_assert(ztl::details::narrow_is_safe<uint8_t>(int32_t(255)), "255 fits in uint8");

    // Unsigned → signed, value within signed range
    static_assert(ztl::details::narrow_is_safe<int>(0u), "0u → int");
    static_assert(ztl::details::narrow_is_safe<int>(static_cast<unsigned>(std::numeric_limits<int>::max())),
                  "INT_MAX as unsigned → int is safe");

    CHECK(true);
}

// ============================================================================
//  narrow — constexpr safe conversions
// ============================================================================

TEST_CASE("narrow returns correct values for safe conversions at compile time") {
    constexpr int a = ztl::narrow<int>(42);
    static_assert(a == 42, "identity");

    constexpr int b = ztl::narrow<int>(42ULL);
    static_assert(b == 42, "ULL → int");

    constexpr int16_t c = ztl::narrow<int16_t>(int32_t(32767));
    static_assert(c == 32767, "INT16_MAX boundary");

    constexpr int16_t d = ztl::narrow<int16_t>(int32_t(-32768));
    static_assert(d == -32768, "INT16_MIN boundary");

    constexpr unsigned e = ztl::narrow<unsigned>(42);
    static_assert(e == 42u, "int → unsigned (positive)");

    constexpr int f = ztl::narrow<int>(0u);
    static_assert(f == 0, "unsigned zero → int");

    constexpr uint8_t g = ztl::narrow<uint8_t>(255U);
    static_assert(g == 255, "UINT8_MAX boundary");

    constexpr int8_t h = ztl::narrow<int8_t>(0LL);
    static_assert(h == 0, "zero from int64");

    CHECK(a == 42);
    CHECK(b == 42);
    CHECK(c == 32767);
    CHECK(d == -32768);
    CHECK(e == 42u);
    CHECK(f == 0);
    CHECK(g == 255);
    CHECK(h == 0);
}

// ============================================================================
//  narrow — runtime safe conversions
// ============================================================================

TEST_CASE("narrow returns correct values for safe conversions at runtime") {
    volatile unsigned long long x = 42ULL;
    CHECK(ztl::narrow<int>(static_cast<unsigned long long>(x)) == 42);

    volatile int y = 100;
    CHECK(ztl::narrow<int8_t>(static_cast<int>(y)) == 100);

    volatile int z = 0;
    CHECK(ztl::narrow<unsigned>(static_cast<int>(z)) == 0u);

    volatile int32_t w = -128;
    CHECK(ztl::narrow<int8_t>(static_cast<int32_t>(w)) == -128);

    volatile unsigned u = 255;
    CHECK(ztl::narrow<uint8_t>(static_cast<unsigned>(u)) == 255);
}

// ============================================================================
//  narrow — runtime trap verification via subprocess (POSIX only)
// ============================================================================
//  __builtin_trap() raises SIGILL which cannot be recovered from in-process.
//  We fork a child process and verify it was killed by a signal.

#if (defined(__unix__) || defined(__APPLE__)) && !defined(ZTL_NARROW_SKIP_TRAP_TESTS)
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>

namespace {
template <typename F>
bool child_was_killed(F&& f) {
    // Flush before forking so parent/child don't duplicate buffered output
    fflush(nullptr);
    pid_t pid = fork();
    if (pid == 0) {
        // Child: reset signal handlers that doctest may have installed,
        // then run the test and exit cleanly if it doesn't trap.
        signal(SIGILL, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
        f();
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFSIGNALED(status);
}
} // namespace

TEST_CASE("narrow traps at runtime when value is truncated") {
    CHECK(child_was_killed([] {
        volatile unsigned long long big = 5000000000ULL;
        ztl::narrow<int>(static_cast<unsigned long long>(big));
    }));
}

TEST_CASE("narrow traps at runtime when sign changes") {
    CHECK(child_was_killed([] {
        volatile int neg = -1;
        ztl::narrow<unsigned>(static_cast<int>(neg));
    }));
}

TEST_CASE("narrow traps at runtime when int8 overflows") {
    CHECK(child_was_killed([] {
        volatile int32_t big = 200;
        ztl::narrow<int8_t>(static_cast<int32_t>(big));
    }));
}

#endif // POSIX trap tests

// ============================================================================
//  UDL integration — narrow is used inside unit literals
// ============================================================================

TEST_CASE("unit literals use narrow for safe construction") {
    using namespace ztl::units;

    CHECK((42_V).count() == 42);
    CHECK((1000_mA).count() == 1000);
    CHECK((100_us).count() == 100);
    CHECK((360_Deg).count() == 360);
    CHECK((9_N).count() == 9);
    CHECK((50_mm).count() == 50);

    // constexpr usage through literals
    constexpr auto cv = 3300_mV;
    static_assert(cv.count() == 3300, "constexpr literal must work");
}
