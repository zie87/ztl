#include <ztl/functional/function_ref.hpp>
#include <ztl/functional/inplace_function.hpp>

#include <doctest/doctest.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace {

int free_function_no_args() noexcept {
    return 123;
}

int add(int a, int b) {
    return a + b;
}

int negate(int x) {
    return -x;
}

short short_result() {
    return 38;
}

struct counter {
    int add(int value) noexcept { return total += value; }
    int get() const noexcept { return total; }

    int total;
};

// Tracks the number of live instances to verify construction/destruction pairing.
struct tracked {
    static int live;
    static int copies;
    static int moves;

    explicit tracked(int v) noexcept : value(v) { ++live; }
    tracked(const tracked& other) noexcept : value(other.value) {
        ++live;
        ++copies;
    }
    tracked(tracked&& other) noexcept : value(other.value) {
        ++live;
        ++moves;
    }
    tracked& operator=(const tracked&) = delete;
    ~tracked() { --live; }

    int operator()(int x) const noexcept { return value + x; }

    static void reset_counters() noexcept {
        live   = 0;
        copies = 0;
        moves  = 0;
    }

    int value;
};

int tracked::live   = 0;
int tracked::copies = 0;
int tracked::moves  = 0;

struct move_only_callable {
    move_only_callable()                          = default;
    move_only_callable(move_only_callable&&)      = default;
    move_only_callable(const move_only_callable&) = delete;
    int operator()() const noexcept { return 0; }
};

struct big_callable {
    unsigned char payload[64];
    int           operator()() const noexcept { return payload[0]; }
};

struct incompatible_callable {
    int operator()(const char*) const noexcept { return 0; }
};

using fn_t = ztl::inplace_function<int(int)>;

} // namespace

TEST_CASE("inplace_function has std::function-like value semantics") {
    static_assert(std::is_nothrow_default_constructible<fn_t>::value, "default construction yields an empty wrapper");
    static_assert(std::is_copy_constructible<fn_t>::value, "copyable like std::function");
    static_assert(std::is_copy_assignable<fn_t>::value, "copy assignable like std::function");
    static_assert(std::is_nothrow_move_constructible<fn_t>::value, "move construction is noexcept");
    static_assert(std::is_nothrow_move_assignable<fn_t>::value, "move assignment is noexcept");
    static_assert(std::is_same<fn_t::result_type, int>::value, "result_type is exposed");
    static_assert(fn_t::capacity == ZTL_INPLACE_FUNCTION_DEFAULT_CAPACITY, "default capacity");
    static_assert(std::is_same<decltype(std::declval<fn_t&>() = std::declval<fn_t const&>()), fn_t&>::value,
                  "assignment returns the derived wrapper");
    static_assert(std::is_same<decltype(std::declval<fn_t&>() = &negate), fn_t&>::value,
                  "converting assignment returns the derived wrapper");
    static_assert(std::is_same<decltype(std::declval<fn_t&>() = nullptr), fn_t&>::value,
                  "nullptr assignment returns the derived wrapper");
}

TEST_CASE("inplace_function rejects unsuitable targets") {
    static_assert(!std::is_constructible<fn_t, incompatible_callable>::value, "wrong call signature");
    static_assert(!std::is_constructible<fn_t, int>::value, "not callable");
    static_assert(!std::is_constructible<ztl::inplace_function<int()>, move_only_callable>::value,
                  "targets must be copy constructible");
    static_assert(std::is_constructible<ztl::inplace_function<int()>, big_callable>::value,
                  "oversized targets are diagnosed by static_assert, not overload resolution");
}

TEST_CASE("inplace_function empty state") {
    fn_t empty;
    CHECK_FALSE(empty);
    CHECK(empty == nullptr);
    CHECK(nullptr == empty);
    CHECK_FALSE(empty != nullptr);

    fn_t from_nullptr(nullptr);
    CHECK_FALSE(from_nullptr);

    int (*null_function)(int) = nullptr;
    fn_t from_null_pointer(null_function);
    CHECK_FALSE(from_null_pointer);

    int (counter::*null_member)(int) = nullptr;
    ztl::inplace_function<int(counter&, int)> from_null_member(null_member);
    CHECK_FALSE(from_null_member);

    // wrapping a different signature stores the whole source wrapper, so it needs more room
    ztl::inplace_function<long(int), sizeof(fn_t)> from_empty_other_signature(empty);
    CHECK_FALSE(from_empty_other_signature);
}

TEST_CASE("inplace_function stores and calls targets") {
    SUBCASE("free function") {
        ztl::inplace_function<int(int, int)> fn(add);
        REQUIRE(fn);
        CHECK(fn(2, 3) == 5);
    }

    SUBCASE("function pointer with converted return type") {
        ztl::inplace_function<long()> fn(&short_result);
        CHECK(fn() == 38L);
    }

    SUBCASE("converting assignment from a function pointer") {
        fn_t fn;
        fn = &negate;
        CHECK(fn(3) == -3);
    }

    SUBCASE("void result discards the return value") {
        ztl::inplace_function<void(int, int)> fn(add);
        fn(1, 2);
    }

    SUBCASE("capturing lambda is copied, not referenced") {
        int  offset = 10;
        fn_t fn([offset](int x) { return x + offset; });
        offset = 1000;
        CHECK(fn(5) == 15);
    }

    SUBCASE("mutable lambda keeps state between calls") {
        ztl::inplace_function<int()> fn([n = 0]() mutable { return ++n; });
        CHECK(fn() == 1);
        CHECK(fn() == 2);
        const auto& cref = fn;
        CHECK(cref() == 3);
    }

    SUBCASE("member function pointer like std::function") {
        counter                                   c = {0};
        ztl::inplace_function<int(counter&, int)> fn(&counter::add);
        CHECK(fn(c, 4) == 4);
        CHECK(c.total == 4);
    }

    SUBCASE("member object pointer") {
        counter                               c = {7};
        ztl::inplace_function<int&(counter&)> fn(&counter::total);
        fn(c) = 9;
        CHECK(c.total == 9);
    }

    SUBCASE("arguments are perfectly forwarded") {
        ztl::inplace_function<void(int&)> fn([](int& v) { v = 42; });
        int                               v = 0;
        fn(v);
        CHECK(v == 42);

        tracked::reset_counters();
        ztl::inplace_function<int(tracked)> by_value([](tracked t) { return t.value; });
        CHECK(by_value(tracked(3)) == 3);
        CHECK(tracked::copies == 0);
        CHECK(tracked::live == 0);
    }
}

TEST_CASE("inplace_function binds compile-time functions and owns bound arguments") {
    SUBCASE("plain function address") {
        ztl::inplace_function<int()> fn(ZTL_NONTYPE(&free_function_no_args));
        CHECK(fn() == 123);
    }

    SUBCASE("bound object is held by value") {
        counter                         c = {1};
        ztl::inplace_function<int(int)> fn(ZTL_NONTYPE(&counter::add), c);
        CHECK(fn(2) == 3);
        CHECK(fn(2) == 5);
        CHECK(c.total == 1);
    }

    SUBCASE("bound pointer binds by reference") {
        counter                         c = {1};
        ztl::inplace_function<int(int)> fn(ZTL_NONTYPE(&counter::add), &c);
        CHECK(fn(2) == 3);
        CHECK(c.total == 3);
    }

    SUBCASE("several bound arguments, bind_front style") {
        ztl::inplace_function<int()> fn(ZTL_NONTYPE(&add), 40, 2);
        CHECK(fn() == 42);
    }

    SUBCASE("bound arguments survive copies of the wrapper") {
        ztl::inplace_function<int()> original(ZTL_NONTYPE(&counter::get), counter{77});
        ztl::inplace_function<int()> copy(original);
        original = nullptr;
        CHECK(copy() == 77);
    }

    SUBCASE("nontype assignment") {
        ztl::inplace_function<int()> fn;
        fn = ZTL_NONTYPE(&free_function_no_args);
        CHECK(fn() == 123);
    }
}

TEST_CASE("inplace_function manages the target lifetime") {
    tracked::reset_counters();
    {
        fn_t a(tracked(1));
        CHECK(tracked::live == 1);

        fn_t b(a);
        CHECK(tracked::live == 2);
        CHECK(b(1) == 2);

        fn_t c(std::move(a));
        CHECK(tracked::live == 2);
        CHECK_FALSE(a);
        CHECK(c(1) == 2);

        b = nullptr;
        CHECK(tracked::live == 1);

        b = c;
        CHECK(tracked::live == 2);

        b = tracked(5);
        CHECK(tracked::live == 2);
        CHECK(b(1) == 6);

        b = fn_t([](int x) { return x; });
        CHECK(tracked::live == 1);

        fn_t& alias = c;
        c           = alias;
        CHECK(tracked::live == 1);
        CHECK(c(0) == 1);

        c = std::move(alias);
        CHECK(tracked::live == 1);
    }
    CHECK(tracked::live == 0);
}

TEST_CASE("inplace_function swap") {
    tracked::reset_counters();
    {
        fn_t a(tracked(1));
        fn_t b([](int x) { return x * 10; });
        fn_t empty;

        a.swap(b);
        CHECK(a(2) == 20);
        CHECK(b(2) == 3);

        swap(b, empty);
        CHECK_FALSE(b);
        CHECK(empty(2) == 3);

        std::swap(a, empty);
        CHECK(a(2) == 3);
        CHECK(empty(2) == 20);

        a.swap(a);
        CHECK(a(2) == 3);
        CHECK(tracked::live == 1);
    }
    CHECK(tracked::live == 0);
}

TEST_CASE("inplace_function target<T>() works without RTTI") {
    fn_t fn(tracked(4));
    REQUIRE(fn.target<tracked>() != nullptr);
    CHECK(fn.target<tracked>()->value == 4);
    CHECK(fn.target<int (*)(int)>() == nullptr);
    CHECK(fn.target<int>() == nullptr);

    const fn_t& cfn = fn;
    static_assert(std::is_same<decltype(cfn.target<tracked>()), const tracked*>::value, "const overload");
    CHECK(cfn.target<tracked>() != nullptr);

    using fp_t = int (*)(int, int);
    ztl::inplace_function<int(int, int)> from_pointer(add);
    REQUIRE(from_pointer.target<fp_t>() != nullptr);
    CHECK(*from_pointer.target<fp_t>() == &add);

    fn_t empty;
    CHECK(empty.target<tracked>() == nullptr);
}

TEST_CASE("inplace_function capacity and alignment are configurable") {
    struct capture {
        std::uint64_t a, b, c, d, e;
    };
    capture cap{1, 2, 3, 4, 5};

    using large_fn = ztl::inplace_function<std::uint64_t(), sizeof(capture), alignof(capture)>;
    large_fn fn([cap]() { return cap.a + cap.b + cap.c + cap.d + cap.e; });
    CHECK(fn() == 15U);

    using tiny_fn = ztl::inplace_function<int(), sizeof(void*), alignof(void*)>;
    static_assert(sizeof(tiny_fn) == 2U * sizeof(void*), "only the vtable pointer is added to the buffer");
    tiny_fn tiny(&free_function_no_args);
    CHECK(tiny() == 123);
}

TEST_CASE("inplace_function converts between capacities without re-wrapping") {
    tracked::reset_counters();
    {
        ztl::inplace_function<int(int), 16, 8>  small(tracked(2));
        ztl::inplace_function<int(int), 64, 16> large(small);
        CHECK(tracked::live == 2);
        REQUIRE(large.target<tracked>() != nullptr);
        CHECK(large(1) == 3);

        ztl::inplace_function<int(int), 64, 16> moved(std::move(small));
        CHECK_FALSE(small);
        CHECK(moved.target<tracked>() != nullptr);

        large = ztl::inplace_function<int(int), 16, 8>(tracked(7));
        CHECK(large(0) == 7);
    }
    CHECK(tracked::live == 0);
}

TEST_CASE("inplace_function wraps other signatures") {
    ztl::inplace_function<short(int)>                narrow([](int x) { return static_cast<short>(x); });
    ztl::inplace_function<long(int), sizeof(narrow)> wide(narrow);
    REQUIRE(wide);
    CHECK(wide(5) == 5L);
}

TEST_CASE("inplace_function interoperates with function_ref") {
    SUBCASE("function_ref references an inplace_function") {
        ztl::inplace_function<int()> fn([n = 0]() mutable { return ++n; });
        ztl::function_ref<int()>     ref(fn);
        CHECK(ref() == 1);
        CHECK(fn() == 2);
        static_assert(!std::is_constructible<ztl::function_ref<int()>, ztl::inplace_function<int()>>::value,
                      "function_ref must not bind a temporary inplace_function");
    }

    SUBCASE("inplace_function stores a function_ref") {
        int                      calls  = 0;
        auto                     lambda = [&calls]() { return ++calls; };
        ztl::function_ref<int()> ref(lambda);

        ztl::inplace_function<int()> fn(ref);
        CHECK(fn() == 1);
        CHECK(calls == 1);
    }

    SUBCASE("same nontype API") {
        counter                         c = {0};
        ztl::function_ref<int(int)>     ref(ZTL_NONTYPE(&counter::add), c);
        ztl::inplace_function<int(int)> fn(ZTL_NONTYPE(&counter::add), &c);
        ref(1);
        fn(2);
        CHECK(c.total == 3);
    }
}

#if ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE
TEST_CASE("inplace_function supports noexcept signatures when the compiler does") {
    using nx_fn = ztl::inplace_function<int() noexcept>;
    nx_fn fn(&free_function_no_args);
    static_assert(noexcept(fn()), "noexcept signature yields a noexcept call operator");
    static_assert(!std::is_constructible<nx_fn, short (*)()>::value, "potentially throwing targets are rejected");
    CHECK(fn() == 123);
}
#endif
