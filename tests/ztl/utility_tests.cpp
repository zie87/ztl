#include "ztl/utility.hpp"

#include <doctest/doctest.h>

#include <type_traits>
#include <utility>

namespace {

struct count_action {
    int* count = nullptr;

    explicit count_action(int& c) noexcept : count(&c) {}
    void operator()() noexcept { ++(*count); }
};

struct move_only_action {
    int* count = nullptr;

    explicit move_only_action(int& c) noexcept : count(&c) {}

    move_only_action(const move_only_action&)            = delete;
    move_only_action& operator=(const move_only_action&) = delete;

    move_only_action(move_only_action&& other) noexcept : count(other.count) { other.count = nullptr; }

    move_only_action& operator=(move_only_action&&) = delete;

    void operator()() noexcept {
        if (count != nullptr) {
            ++(*count);
        }
    }
};

struct mutable_action {
    int* count       = nullptr;
    int  local_count = 0;

    explicit mutable_action(int& c) noexcept : count(&c), local_count(0) {}

    void operator()() noexcept {
        ++local_count;
        *count += local_count;
    }
};

} // namespace

TEST_CASE("ztl::final_action type traits") {
    typedef ztl::final_action<count_action> guard_type;

    static_assert(!std::is_copy_constructible<guard_type>::value, "final_action must not be copy constructible");

    static_assert(!std::is_copy_assignable<guard_type>::value, "final_action must not be copy assignable");

    static_assert(std::is_move_constructible<guard_type>::value, "final_action must be move constructible");

    static_assert(!std::is_move_assignable<guard_type>::value, "final_action must not be move assignable");

    static_assert(std::is_nothrow_destructible<guard_type>::value, "final_action destructor must be noexcept");

    CHECK(true);
}

TEST_CASE("ztl::finally returns a decayed final_action") {
    typedef decltype(ztl::finally(std::declval<count_action&>())) result_type;
    static_assert(std::is_same<result_type, ztl::final_action<count_action>>::value,
                  "finally must decay the callable type");
    CHECK(true);
}

TEST_CASE("ztl::final_action invokes callable when destroyed") {
    int count = 0;
    {
        ztl::final_action<count_action> guard{count_action{count}};
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::final_action can be constructed from lvalue callable") {
    int          count = 0;
    count_action action{count};
    {
        ztl::final_action<count_action> guard{action};
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::finally invokes callable at scope exit") {
    int count = 0;
    {
        auto guard = ztl::finally([&count]() noexcept { ++count; });
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::finally supports mutable callable") {
    int count = 0;
    {
        auto guard = ztl::finally(mutable_action{count});
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::finally supports move-only callable") {
    int count = 0;
    {
        auto guard = ztl::finally(move_only_action{count});
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::final_action move construction transfers ownership") {
    int count = 0;
    {
        auto guard1 = ztl::finally([&count]() noexcept { ++count; });
        CHECK(count == 0);
        {
            auto guard2 = std::move(guard1);
            CHECK(count == 0);
        }
        CHECK(count == 1);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::final_action moved-from object does not invoke callable") {
    int count = 0;
    {
        auto guard1 = ztl::finally([&count]() noexcept { ++count; });
        auto guard2 = std::move(guard1);
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::final_action can be moved multiple times and still invokes once") {
    int count = 0;
    {
        auto guard1 = ztl::finally([&count]() noexcept { ++count; });
        auto guard2 = std::move(guard1);
        auto guard3 = std::move(guard2);
        CHECK(count == 0);
    }
    CHECK(count == 1);
}

TEST_CASE("ztl::final_action executes guards in reverse construction order") {
    int value = 0;
    {
        auto first  = ztl::finally([&value]() noexcept { value = value * 10 + 1; });
        auto second = ztl::finally([&value]() noexcept { value = value * 10 + 2; });
        CHECK(value == 0);
    }

    CHECK(value == 21);
}

TEST_CASE("ztl::final_action works with pointer cleanup style use case") {
    bool resource_acquired = true;
    {
        auto cleanup = ztl::finally([&resource_acquired]() noexcept { resource_acquired = false; });
        CHECK(resource_acquired);
    }
    CHECK_FALSE(resource_acquired);
}
