#include "ztl/ring_buffer.hpp"

#include "doctest/doctest.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

template <std::size_t CAPACITY_V>
struct capacity_case {
    enum { value = CAPACITY_V };
};

struct small_value {
    small_value() noexcept : value{0} {}
    explicit small_value(int v) noexcept : value{v} {}

    bool operator==(const small_value& rhs) const noexcept { return value == rhs.value; }
    bool operator!=(const small_value& rhs) const noexcept { return !(*this == rhs); }

    int value;
};

struct wide_value {
    wide_value() noexcept : id{0}, payload{0U} {}
    explicit wide_value(int v) noexcept : id{v}, payload{static_cast<unsigned>(v * 10)} {}

    bool operator==(const wide_value& rhs) const noexcept { return id == rhs.id && payload == rhs.payload; }
    bool operator!=(const wide_value& rhs) const noexcept { return !(*this == rhs); }

    int      id;
    unsigned payload;
};

struct move_only_value {
    move_only_value() = delete;
    explicit move_only_value(int v) noexcept : value{v} {}

    move_only_value(const move_only_value&)            = delete;
    move_only_value& operator=(const move_only_value&) = delete;

    move_only_value(move_only_value&& rhs) noexcept : value{rhs.value} { rhs.value = -1; }

    move_only_value& operator=(move_only_value&& rhs) noexcept {
        value     = rhs.value;
        rhs.value = -1;
        return *this;
    }

    bool operator==(const move_only_value& rhs) const noexcept { return value == rhs.value; }
    bool operator!=(const move_only_value& rhs) const noexcept { return !(*this == rhs); }

    int value;
};

struct non_default_value {
    non_default_value() = delete;
    explicit non_default_value(int v) noexcept : value{v} {}

    non_default_value(const non_default_value&) noexcept = default;
    non_default_value(non_default_value&&) noexcept      = default;

    non_default_value& operator=(const non_default_value&) noexcept = default;
    non_default_value& operator=(non_default_value&&) noexcept      = default;

    bool operator==(const non_default_value& rhs) const noexcept { return value == rhs.value; }
    bool operator!=(const non_default_value& rhs) const noexcept { return !(*this == rhs); }

    int value;
};

struct non_assignable_value {
    non_assignable_value() = delete;
    explicit non_assignable_value(int v) noexcept : value{v} {}

    non_assignable_value(const non_assignable_value&) noexcept = default;
    non_assignable_value(non_assignable_value&&) noexcept      = default;

    non_assignable_value& operator=(const non_assignable_value&) = delete;
    non_assignable_value& operator=(non_assignable_value&&)      = delete;

    bool operator==(const non_assignable_value& rhs) const noexcept { return value == rhs.value; }
    bool operator!=(const non_assignable_value& rhs) const noexcept { return !(*this == rhs); }

    int value;
};

struct pair_value {
    pair_value() = delete;
    pair_value(int first, int second) noexcept : first{first}, second{second} {}

    bool operator==(const pair_value& rhs) const noexcept { return first == rhs.first && second == rhs.second; }
    bool operator!=(const pair_value& rhs) const noexcept { return !(*this == rhs); }

    int first;
    int second;
};

struct tracked_value {
    tracked_value() = delete;

    explicit tracked_value(int v) noexcept : value{v} {
        ++live;
        ++constructed;
    }

    tracked_value(const tracked_value& rhs) noexcept : value{rhs.value} {
        ++live;
        ++constructed;
        ++copied;
    }

    tracked_value(tracked_value&& rhs) noexcept : value{rhs.value} {
        rhs.value = -1;
        ++live;
        ++constructed;
        ++moved;
    }

    tracked_value& operator=(const tracked_value&) = delete;
    tracked_value& operator=(tracked_value&&)      = delete;

    ~tracked_value() noexcept {
        --live;
        ++destroyed;
    }

    bool operator==(const tracked_value& rhs) const noexcept { return value == rhs.value; }
    bool operator!=(const tracked_value& rhs) const noexcept { return !(*this == rhs); }

    int value;

    static int live;
    static int constructed;
    static int destroyed;
    static int copied;
    static int moved;
};

int tracked_value::live        = 0;
int tracked_value::constructed = 0;
int tracked_value::destroyed   = 0;
int tracked_value::copied      = 0;
int tracked_value::moved       = 0;

struct lifetime_flag {
    bool alive;
};

struct alias_checked_value {
    alias_checked_value() = delete;

    alias_checked_value(int v, lifetime_flag& f) noexcept : value{v}, flag{&f} { flag->alive = true; }

    alias_checked_value(const alias_checked_value& rhs) noexcept : value{rhs.value}, flag{rhs.flag} {
        copied_from_destroyed = copied_from_destroyed || !rhs.flag->alive;
    }

    alias_checked_value(alias_checked_value&& rhs) noexcept : value{rhs.value}, flag{rhs.flag} {
        copied_from_destroyed = copied_from_destroyed || !rhs.flag->alive;
        rhs.value             = -1;
    }

    alias_checked_value& operator=(const alias_checked_value&) = delete;
    alias_checked_value& operator=(alias_checked_value&&)      = delete;

    ~alias_checked_value() noexcept {
        if (flag != nullptr) {
            flag->alive = false;
        }
    }

    int            value;
    lifetime_flag* flag;

    static bool copied_from_destroyed;
};

bool alias_checked_value::copied_from_destroyed = false;

struct mutating_reject_policy {
    static bool on_overflow(std::size_t& tail, std::size_t slots) noexcept {
        tail = (tail + 1U) % slots;
        return false;
    }
};

struct offset_popper {
    explicit offset_popper(int v = 0) noexcept : offset{v} {}

    offset_popper(const offset_popper&) noexcept            = default;
    offset_popper& operator=(const offset_popper&) noexcept = default;

    offset_popper(offset_popper&& rhs) noexcept : offset{rhs.offset} { rhs.offset = -1000; }

    offset_popper& operator=(offset_popper&& rhs) noexcept {
        offset     = rhs.offset;
        rhs.offset = -1000;
        return *this;
    }

    int operator()(int& value) const noexcept { return value + offset; }

    int offset;
};

} // namespace

TEST_CASE_TEMPLATE("ring_buffer starts empty and reports compile-time capacity", CAPACITY, capacity_case<1>,
                   capacity_case<3>, capacity_case<5>) {
    typedef ztl::ring_buffer<int, CAPACITY::value> buffer_type;

    buffer_type buffer;

    CHECK(buffer.empty());
    CHECK_FALSE(buffer.full());
    CHECK(buffer.size() == 0U);
    CHECK(buffer.capacity() == CAPACITY::value);
    CHECK(buffer_type::capacity() == CAPACITY::value);
    CHECK(buffer.begin() == buffer.end());
    CHECK(buffer.cbegin() == buffer.cend());
    CHECK(std::distance(buffer.begin(), buffer.end()) == 0);
}

TEST_CASE_TEMPLATE("try_push_back fills the buffer and rejects one extra element", CAPACITY, capacity_case<1>,
                   capacity_case<3>, capacity_case<5>) {
    typedef ztl::ring_buffer<int, CAPACITY::value> buffer_type;

    buffer_type buffer;

    for (std::size_t i = 0U; i < CAPACITY::value; ++i) {
        CHECK(buffer.try_push_back(static_cast<int>(i + 1U)));
        CHECK(buffer.size() == i + 1U);
    }

    CHECK(buffer.full());
    CHECK(buffer.size() == CAPACITY::value);
    CHECK_FALSE(buffer.try_push_back(99));
    CHECK(buffer.size() == CAPACITY::value);

    for (std::size_t i = 0U; i < CAPACITY::value; ++i) {
        CHECK(buffer[i] == static_cast<int>(i + 1U));
    }
}

TEST_CASE_TEMPLATE("push, emplace and access preserve logical order for different value types", T, int, std::uint16_t,
                   small_value, wide_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type buffer;

    const T copied_value(1);
    buffer.push_back(copied_value);
    buffer.push_back(T(2));
    buffer.emplace_back(3);
    CHECK(buffer.try_emplace_back(4));
    CHECK_FALSE(buffer.try_emplace_back(5));

    CHECK(buffer.full());
    CHECK(buffer.size() == 4U);
    CHECK((buffer.front() == T(1)));
    CHECK((buffer.back() == T(4)));
    CHECK((buffer[0] == T(1)));
    CHECK((buffer[1] == T(2)));
    CHECK((buffer[2] == T(3)));
    CHECK((buffer[3] == T(4)));

    buffer.front() = T(10);
    buffer[1]      = T(20);
    buffer.back()  = T(40);

    CHECK((buffer[0] == T(10)));
    CHECK((buffer[1] == T(20)));
    CHECK((buffer[2] == T(3)));
    CHECK((buffer[3] == T(40)));
}

TEST_CASE("ring_buffer supports non-default-constructible move-only values for rvalue insertion") {
    ztl::ring_buffer<move_only_value, 2> buffer;

    buffer.push_back(move_only_value(1));
    buffer.emplace_back(2);

    CHECK(buffer.full());
    CHECK((buffer.front() == move_only_value(1)));
    CHECK((buffer.back() == move_only_value(2)));

    move_only_value first = buffer.pop_front();
    CHECK((first == move_only_value(1)));
    CHECK((buffer.front() == move_only_value(2)));
}

TEST_CASE("ring_buffer storage does not require default construction of T") {
    static_assert(!std::is_default_constructible<non_default_value>::value,
                  "non_default_value is intentionally not default-constructible");
    static_assert(std::is_default_constructible<ztl::ring_buffer<non_default_value, 2>>::value,
                  "raw-storage ring_buffer should be default-constructible even when T is not");

    ztl::ring_buffer<non_default_value, 2> buffer;

    CHECK(buffer.empty());
    buffer.emplace_back(1);
    CHECK(buffer.try_emplace_back(2));
    CHECK_FALSE(buffer.try_emplace_back(3));

    CHECK(buffer.full());
    CHECK((buffer.front() == non_default_value(1)));
    CHECK((buffer.back() == non_default_value(2)));
}

TEST_CASE("ring_buffer insertion does not require assignment into an existing slot") {
    static_assert(!std::is_default_constructible<non_assignable_value>::value,
                  "non_assignable_value is intentionally not default-constructible");
    static_assert(!std::is_assignable<non_assignable_value&, non_assignable_value>::value,
                  "non_assignable_value is intentionally not assignable");

    ztl::ring_buffer<non_assignable_value, 3> buffer;

    buffer.emplace_back(1);
    buffer.push_back(non_assignable_value(2));
    CHECK(buffer.try_push_back(non_assignable_value(3)));
    CHECK_FALSE(buffer.try_emplace_back(4));

    CHECK(buffer.full());
    CHECK((buffer[0] == non_assignable_value(1)));
    CHECK((buffer[1] == non_assignable_value(2)));
    CHECK((buffer[2] == non_assignable_value(3)));

    non_assignable_value removed = buffer.pop_front();
    CHECK((removed == non_assignable_value(1)));
    CHECK((buffer.front() == non_assignable_value(2)));
}

TEST_CASE("emplace_back constructs non-default values from multiple arguments") {
    ztl::ring_buffer<pair_value, 2> buffer;

    buffer.emplace_back(1, 10);
    CHECK(buffer.try_emplace_back(2, 20));
    CHECK_FALSE(buffer.try_emplace_back(3, 30));

    CHECK((buffer.front() == pair_value(1, 10)));
    CHECK((buffer.back() == pair_value(2, 20)));
}

TEST_CASE_TEMPLATE("push_back with overwrite policy keeps the newest values", CAPACITY, capacity_case<1>,
                   capacity_case<3>, capacity_case<5>) {
    typedef ztl::ring_buffer<int, CAPACITY::value> buffer_type;

    buffer_type buffer;

    for (std::size_t i = 0U; i < CAPACITY::value; ++i) {
        buffer.push_back(static_cast<int>(i + 1U));
    }

    buffer.push_back(99);

    CHECK(buffer.full());
    CHECK(buffer.size() == CAPACITY::value);

    if (CAPACITY::value == 1U) {
        CHECK(buffer[0] == 99);
    } else {
        for (std::size_t i = 0U; i + 1U < CAPACITY::value; ++i) {
            CHECK(buffer[i] == static_cast<int>(i + 2U));
        }
        CHECK(buffer[CAPACITY::value - 1U] == 99);
    }
}

TEST_CASE_TEMPLATE("push_back with discard policy leaves a full buffer unchanged", CAPACITY, capacity_case<1>,
                   capacity_case<3>, capacity_case<5>) {
    typedef ztl::ring_buffer<int, CAPACITY::value, ztl::default_popper<int>, ztl::discard_policy> buffer_type;

    buffer_type buffer;

    for (std::size_t i = 0U; i < CAPACITY::value; ++i) {
        buffer.push_back(static_cast<int>(i + 1U));
    }

    buffer.push_back(99);

    CHECK(buffer.full());
    CHECK(buffer.size() == CAPACITY::value);

    for (std::size_t i = 0U; i < CAPACITY::value; ++i) {
        CHECK(buffer[i] == static_cast<int>(i + 1U));
    }
}

TEST_CASE("overwrite push constructs from aliased front before destroying overwritten slot") {
    lifetime_flag first{false};
    lifetime_flag second{false};

    alias_checked_value::copied_from_destroyed = false;

    ztl::ring_buffer<alias_checked_value, 2> buffer;
    buffer.emplace_back(1, first);
    buffer.emplace_back(2, second);

    CHECK(buffer.full());
    CHECK(first.alive);
    CHECK(second.alive);

    buffer.push_back(buffer.front());

    CHECK_FALSE(alias_checked_value::copied_from_destroyed);
    CHECK(buffer.full());
    CHECK(buffer.size() == 2U);
    CHECK(buffer[0].value == 2);
    CHECK(buffer[1].value == 1);
    CHECK_FALSE(first.alive);
    CHECK(second.alive);
}

TEST_CASE("overwrite emplace constructs from aliased front before destroying overwritten slot") {
    lifetime_flag first{false};
    lifetime_flag second{false};

    alias_checked_value::copied_from_destroyed = false;

    ztl::ring_buffer<alias_checked_value, 2> buffer;
    buffer.emplace_back(1, first);
    buffer.emplace_back(2, second);

    CHECK(buffer.full());
    CHECK(first.alive);
    CHECK(second.alive);

    buffer.emplace_back(buffer.front());

    CHECK_FALSE(alias_checked_value::copied_from_destroyed);
    CHECK(buffer.full());
    CHECK(buffer.size() == 2U);
    CHECK(buffer[0].value == 2);
    CHECK(buffer[1].value == 1);
    CHECK_FALSE(first.alive);
    CHECK(second.alive);
}

TEST_CASE("rejecting overflow policy cannot mutate the visible buffer state") {
    typedef ztl::ring_buffer<int, 2, ztl::default_popper<int>, mutating_reject_policy> buffer_type;

    buffer_type buffer;

    buffer.push_back(1);
    buffer.push_back(2);

    CHECK(buffer.full());

    buffer.push_back(3);

    CHECK(buffer.full());
    CHECK(buffer.size() == 2U);
    CHECK(buffer[0] == 1);
    CHECK(buffer[1] == 2);
}

TEST_CASE_TEMPLATE("pop_front and pop_back remove values from the expected side", T, int, std::uint16_t, small_value,
                   wide_value) {
    ztl::ring_buffer<T, 4> buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.push_back(T(4));

    CHECK((buffer.pop_front() == T(1)));
    CHECK(buffer.size() == 3U);
    CHECK((buffer.front() == T(2)));
    CHECK((buffer.back() == T(4)));

    CHECK((buffer.pop_back() == T(4)));
    CHECK(buffer.size() == 2U);
    CHECK((buffer.front() == T(2)));
    CHECK((buffer.back() == T(3)));

    CHECK((buffer.pop_front() == T(2)));
    CHECK((buffer.pop_front() == T(3)));
    CHECK(buffer.empty());
}

TEST_CASE_TEMPLATE("wrap-around keeps FIFO order for different value types", T, int, std::uint16_t, small_value,
                   wide_value) {
    ztl::ring_buffer<T, 4> buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.push_back(T(4));

    CHECK((buffer.pop_front() == T(1)));
    CHECK((buffer.pop_front() == T(2)));

    buffer.push_back(T(5));
    buffer.push_back(T(6));

    CHECK(buffer.full());
    CHECK(buffer.size() == 4U);
    CHECK((buffer[0] == T(3)));
    CHECK((buffer[1] == T(4)));
    CHECK((buffer[2] == T(5)));
    CHECK((buffer[3] == T(6)));

    typename ztl::ring_buffer<T, 4>::iterator it = buffer.begin();
    CHECK((*it == T(3)));
    ++it;
    CHECK((*it == T(4)));
    ++it;
    CHECK((*it == T(5)));
    ++it;
    CHECK((*it == T(6)));
    ++it;
    CHECK(it == buffer.end());
}

TEST_CASE_TEMPLATE("clear remove all visible elements and allow reuse", T, int, std::uint16_t, small_value,
                   wide_value) {
    ztl::ring_buffer<T, 4> buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.clear();

    CHECK(buffer.empty());
    CHECK(buffer.size() == 0U);
    CHECK(buffer.begin() == buffer.end());

    buffer.push_back(T(4));
    buffer.push_back(T(5));

    CHECK(buffer.size() == 2U);
    CHECK((buffer[0] == T(4)));
    CHECK((buffer[1] == T(5)));

    CHECK((buffer.pop_front() == T(4)));
    buffer.clear();

    CHECK(buffer.empty());
    CHECK(buffer.size() == 0U);
    CHECK(buffer.begin() == buffer.end());

    buffer.push_back(T(7));
    buffer.push_back(T(8));

    CHECK(buffer.size() == 2U);
    CHECK((buffer[0] == T(7)));
    CHECK((buffer[1] == T(8)));
}

TEST_CASE_TEMPLATE("swap exchanges logical content after wrap-around", T, int, std::uint16_t, small_value, wide_value) {
    ztl::ring_buffer<T, 4> left;
    ztl::ring_buffer<T, 4> right;

    left.push_back(T(1));
    left.push_back(T(2));
    left.push_back(T(3));
    CHECK((left.pop_front() == T(1)));
    left.push_back(T(4));

    right.push_back(T(10));
    right.push_back(T(20));

    swap(left, right);

    CHECK(left.size() == 2U);
    CHECK((left[0] == T(10)));
    CHECK((left[1] == T(20)));

    CHECK(right.size() == 3U);
    CHECK((right[0] == T(2)));
    CHECK((right[1] == T(3)));
    CHECK((right[2] == T(4)));
}

TEST_CASE_TEMPLATE("ring_buffer_base copy construction preserves logical order after wrap-around", T, int,
                   std::uint16_t, small_value, wide_value, non_assignable_value) {
    typedef ztl::ring_buffer_base<T, 4> buffer_type;

    buffer_type            source;
    ztl::default_popper<T> popper;

    source.template emplace_back<ztl::overwrite_policy>(1);
    source.template emplace_back<ztl::overwrite_policy>(2);
    source.template emplace_back<ztl::overwrite_policy>(3);
    source.template emplace_back<ztl::overwrite_policy>(4);

    CHECK((source.pop_front(popper) == T(1)));
    source.template emplace_back<ztl::overwrite_policy>(5);

    const buffer_type copied{source};

    CHECK(copied.full());
    CHECK(copied.size() == 4U);
    CHECK((copied[0] == T(2)));
    CHECK((copied[1] == T(3)));
    CHECK((copied[2] == T(4)));
    CHECK((copied[3] == T(5)));

    CHECK((source.pop_front(popper) == T(2)));
    source.template emplace_back<ztl::overwrite_policy>(6);

    CHECK(copied.full());
    CHECK(copied.size() == 4U);
    CHECK((copied[0] == T(2)));
    CHECK((copied[1] == T(3)));
    CHECK((copied[2] == T(4)));
    CHECK((copied[3] == T(5)));
    CHECK(source.full());
    CHECK((source[0] == T(3)));
    CHECK((source[1] == T(4)));
    CHECK((source[2] == T(5)));
    CHECK((source[3] == T(6)));
}

TEST_CASE_TEMPLATE("ring_buffer_base copy assignment reconstructs destination from source", T, int, std::uint16_t,
                   small_value, wide_value, non_assignable_value) {
    typedef ztl::ring_buffer_base<T, 4> buffer_type;

    buffer_type            source;
    buffer_type            destination;
    ztl::default_popper<T> popper;

    source.template emplace_back<ztl::overwrite_policy>(1);
    source.template emplace_back<ztl::overwrite_policy>(2);
    source.template emplace_back<ztl::overwrite_policy>(3);
    source.template emplace_back<ztl::overwrite_policy>(4);
    CHECK((source.pop_front(popper) == T(1)));
    source.template emplace_back<ztl::overwrite_policy>(5);

    destination.template emplace_back<ztl::overwrite_policy>(90);
    destination.template emplace_back<ztl::overwrite_policy>(91);

    destination = source;

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    source.clear();

    CHECK(source.empty());
    CHECK(destination.full());
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    buffer_type& same_destination = destination;
    destination                   = same_destination;

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));
}

TEST_CASE_TEMPLATE("ring_buffer_base move construction preserves logical order and clears the source", T,
                   move_only_value, non_assignable_value) {
    typedef ztl::ring_buffer_base<T, 4> buffer_type;

    buffer_type            source;
    ztl::default_popper<T> popper;

    source.template emplace_back<ztl::overwrite_policy>(1);
    source.template emplace_back<ztl::overwrite_policy>(2);
    source.template emplace_back<ztl::overwrite_policy>(3);
    source.template emplace_back<ztl::overwrite_policy>(4);
    CHECK((source.pop_front(popper) == T(1)));
    source.template emplace_back<ztl::overwrite_policy>(5);

    buffer_type moved{std::move(source)};

    CHECK(source.empty());
    CHECK(source.size() == 0U);
    CHECK(moved.full());
    CHECK(moved.size() == 4U);
    CHECK((moved[0] == T(2)));
    CHECK((moved[1] == T(3)));
    CHECK((moved[2] == T(4)));
    CHECK((moved[3] == T(5)));
}

TEST_CASE_TEMPLATE("ring_buffer_base move assignment reconstructs destination and clears the source", T,
                   move_only_value, non_assignable_value) {
    typedef ztl::ring_buffer_base<T, 4> buffer_type;

    buffer_type            source;
    buffer_type            destination;
    ztl::default_popper<T> popper;

    source.template emplace_back<ztl::overwrite_policy>(1);
    source.template emplace_back<ztl::overwrite_policy>(2);
    source.template emplace_back<ztl::overwrite_policy>(3);
    source.template emplace_back<ztl::overwrite_policy>(4);
    CHECK((source.pop_front(popper) == T(1)));
    source.template emplace_back<ztl::overwrite_policy>(5);

    destination.template emplace_back<ztl::overwrite_policy>(90);
    destination.template emplace_back<ztl::overwrite_policy>(91);

    destination = std::move(source);

    CHECK(source.empty());
    CHECK(source.size() == 0U);
    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    buffer_type& same_destination = destination;
    destination                   = std::move(same_destination);

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));
}

TEST_CASE_TEMPLATE("ring_buffer copy construction preserves wrapper state and logical order", T, int, std::uint16_t,
                   small_value, wide_value, non_assignable_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type source;

    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);
    source.emplace_back(4);
    CHECK((source.pop_front() == T(1)));
    source.emplace_back(5);

    const buffer_type copied{source};

    CHECK(copied.full());
    CHECK(copied.size() == 4U);
    CHECK((copied[0] == T(2)));
    CHECK((copied[1] == T(3)));
    CHECK((copied[2] == T(4)));
    CHECK((copied[3] == T(5)));

    CHECK((source.pop_front() == T(2)));
    source.emplace_back(6);

    CHECK(copied.full());
    CHECK((copied[0] == T(2)));
    CHECK((copied[1] == T(3)));
    CHECK((copied[2] == T(4)));
    CHECK((copied[3] == T(5)));
    CHECK(source.full());
    CHECK((source[0] == T(3)));
    CHECK((source[1] == T(4)));
    CHECK((source[2] == T(5)));
    CHECK((source[3] == T(6)));
}

TEST_CASE_TEMPLATE("ring_buffer copy assignment reconstructs destination from source", T, int, std::uint16_t,
                   small_value, wide_value, non_assignable_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type source;
    buffer_type destination;

    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);
    source.emplace_back(4);
    CHECK((source.pop_front() == T(1)));
    source.emplace_back(5);

    destination.emplace_back(90);
    destination.emplace_back(91);

    destination = source;

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    source.clear();

    CHECK(source.empty());
    CHECK(destination.full());
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    buffer_type& same_destination = destination;
    destination                   = same_destination;

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));
}

TEST_CASE_TEMPLATE("ring_buffer move construction preserves logical order and clears the source", T, move_only_value,
                   non_assignable_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type source;

    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);
    source.emplace_back(4);
    CHECK((source.pop_front() == T(1)));
    source.emplace_back(5);

    buffer_type moved{std::move(source)};

    CHECK(source.empty());
    CHECK(source.size() == 0U);
    CHECK(moved.full());
    CHECK(moved.size() == 4U);
    CHECK((moved[0] == T(2)));
    CHECK((moved[1] == T(3)));
    CHECK((moved[2] == T(4)));
    CHECK((moved[3] == T(5)));
}

TEST_CASE_TEMPLATE("ring_buffer move assignment reconstructs destination and clears the source", T, move_only_value,
                   non_assignable_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type source;
    buffer_type destination;

    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);
    source.emplace_back(4);
    CHECK((source.pop_front() == T(1)));
    source.emplace_back(5);

    destination.emplace_back(90);
    destination.emplace_back(91);

    destination = std::move(source);

    CHECK(source.empty());
    CHECK(source.size() == 0U);
    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));

    buffer_type& same_destination = destination;
    destination                   = std::move(same_destination);

    CHECK(destination.full());
    CHECK(destination.size() == 4U);
    CHECK((destination[0] == T(2)));
    CHECK((destination[1] == T(3)));
    CHECK((destination[2] == T(4)));
    CHECK((destination[3] == T(5)));
}

TEST_CASE("ring_buffer copy and move special members preserve the stored popper") {
    typedef ztl::ring_buffer<int, 3, offset_popper> buffer_type;

    buffer_type copied_source{offset_popper{10}};
    copied_source.push_back(1);
    copied_source.push_back(2);

    buffer_type copied_constructed{copied_source};
    CHECK(copied_constructed.pop_front() == 11);
    CHECK(copied_constructed.pop_front() == 12);
    CHECK(copied_constructed.empty());

    buffer_type copied_assigned{offset_popper{20}};
    copied_assigned.push_back(90);
    copied_assigned = copied_source;
    CHECK(copied_assigned.pop_front() == 11);
    CHECK(copied_assigned.pop_front() == 12);
    CHECK(copied_assigned.empty());

    buffer_type move_constructed_source{offset_popper{30}};
    move_constructed_source.push_back(3);
    move_constructed_source.push_back(4);

    buffer_type move_constructed{std::move(move_constructed_source)};
    CHECK(move_constructed_source.empty());
    CHECK(move_constructed.pop_front() == 33);
    CHECK(move_constructed.pop_front() == 34);
    CHECK(move_constructed.empty());

    buffer_type move_assigned_source{offset_popper{40}};
    move_assigned_source.push_back(5);
    move_assigned_source.push_back(6);

    buffer_type move_assigned{offset_popper{50}};
    move_assigned.push_back(90);
    move_assigned = std::move(move_assigned_source);
    CHECK(move_assigned_source.empty());
    CHECK(move_assigned.pop_front() == 45);
    CHECK(move_assigned.pop_front() == 46);
    CHECK(move_assigned.empty());
}

TEST_CASE("ring_buffer_base copy and move special members manage raw-storage lifetimes") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    {
        ztl::ring_buffer_base<tracked_value, 3> source;
        source.template emplace_back<ztl::overwrite_policy>(1);
        source.template emplace_back<ztl::overwrite_policy>(2);

        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);

        {
            ztl::ring_buffer_base<tracked_value, 3> copied{source};

            CHECK(copied.size() == 2U);
            CHECK(copied[0].value == 1);
            CHECK(copied[1].value == 2);
            CHECK(tracked_value::live == 4);
            CHECK(tracked_value::constructed == 4);
            CHECK(tracked_value::destroyed == 0);
            CHECK(tracked_value::copied == 2);
            CHECK(tracked_value::moved == 0);
        }

        CHECK(source.size() == 2U);
        CHECK(source[0].value == 1);
        CHECK(source[1].value == 2);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 4);
        CHECK(tracked_value::destroyed == 2);
        CHECK(tracked_value::copied == 2);
        CHECK(tracked_value::moved == 0);
    }

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 4);
    CHECK(tracked_value::destroyed == 4);
    CHECK(tracked_value::copied == 2);
    CHECK(tracked_value::moved == 0);

    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    {
        ztl::ring_buffer_base<tracked_value, 3> source;
        source.template emplace_back<ztl::overwrite_policy>(3);
        source.template emplace_back<ztl::overwrite_policy>(4);

        ztl::ring_buffer_base<tracked_value, 3> moved{std::move(source)};

        CHECK(source.empty());
        CHECK(moved.size() == 2U);
        CHECK(moved[0].value == 3);
        CHECK(moved[1].value == 4);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 4);
        CHECK(tracked_value::destroyed == 2);
        CHECK(tracked_value::copied == 0);
        CHECK(tracked_value::moved == 2);
    }

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 4);
    CHECK(tracked_value::destroyed == 4);
    CHECK(tracked_value::copied == 0);
    CHECK(tracked_value::moved == 2);
}

TEST_CASE("raw storage does not construct elements until insertion") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    {
        ztl::ring_buffer<tracked_value, 3> buffer;

        CHECK(buffer.empty());
        CHECK(tracked_value::live == 0);
        CHECK(tracked_value::constructed == 0);
        CHECK(tracked_value::destroyed == 0);

        buffer.emplace_back(1);
        CHECK(tracked_value::live == 1);
        CHECK(tracked_value::constructed == 1);
        CHECK(tracked_value::destroyed == 0);

        buffer.emplace_back(2);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);
    }

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 2);
    CHECK(tracked_value::destroyed == 2);
}

TEST_CASE("clear and clear_and_reset destroy only live raw-storage elements") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    ztl::ring_buffer<tracked_value, 3> buffer;

    buffer.emplace_back(1);
    buffer.emplace_back(2);
    buffer.emplace_back(3);

    CHECK(tracked_value::live == 3);
    CHECK(tracked_value::constructed == 3);
    CHECK(tracked_value::destroyed == 0);

    buffer.clear();

    CHECK(buffer.empty());
    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 3);
    CHECK(tracked_value::destroyed == 3);

    buffer.clear();

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 3);
    CHECK(tracked_value::destroyed == 3);

    buffer.emplace_back(4);

    CHECK(buffer.size() == 1U);
    CHECK(buffer.front().value == 4);
    CHECK(tracked_value::live == 1);
    CHECK(tracked_value::constructed == 4);
    CHECK(tracked_value::destroyed == 3);

    buffer.clear();

    CHECK(buffer.empty());
    CHECK(buffer.begin() == buffer.end());
    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 4);
    CHECK(tracked_value::destroyed == 4);
}

TEST_CASE("overwrite policy destroys the overwritten raw-storage element") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    {
        ztl::ring_buffer<tracked_value, 2> buffer;

        buffer.emplace_back(1);
        buffer.emplace_back(2);

        CHECK(buffer.full());
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);

        buffer.emplace_back(3);

        CHECK(buffer.full());
        CHECK(buffer.size() == 2U);
        CHECK(buffer.front().value == 2);
        CHECK(buffer.back().value == 3);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 3);
        CHECK(tracked_value::destroyed == 1);
    }

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 3);
    CHECK(tracked_value::destroyed == 3);
}

TEST_CASE("discard policy does not construct or destroy when a raw-storage buffer is full") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    {
        ztl::ring_buffer<tracked_value, 2, ztl::default_popper<tracked_value>, ztl::discard_policy> buffer;

        buffer.emplace_back(1);
        buffer.emplace_back(2);

        CHECK(buffer.full());
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);

        buffer.emplace_back(3);

        CHECK(buffer.full());
        CHECK(buffer.front().value == 1);
        CHECK(buffer.back().value == 2);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);

        CHECK_FALSE(buffer.try_emplace_back(4));

        CHECK(buffer.full());
        CHECK(buffer.front().value == 1);
        CHECK(buffer.back().value == 2);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 2);
        CHECK(tracked_value::destroyed == 0);
    }

    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 2);
    CHECK(tracked_value::destroyed == 2);
}

TEST_CASE("pop_front and pop_back move values out and then destroy raw-storage slots") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    ztl::ring_buffer<tracked_value, 2> buffer;

    buffer.emplace_back(1);
    buffer.emplace_back(2);

    CHECK(tracked_value::live == 2);
    CHECK(tracked_value::constructed == 2);
    CHECK(tracked_value::destroyed == 0);

    {
        tracked_value removed = buffer.pop_front();

        CHECK(removed.value == 1);
        CHECK(buffer.size() == 1U);
        CHECK(buffer.front().value == 2);
        CHECK(tracked_value::live == 2);
        CHECK(tracked_value::constructed == 3);
        CHECK(tracked_value::destroyed == 1);
        CHECK(tracked_value::moved == 1);
    }

    CHECK(buffer.size() == 1U);
    CHECK(tracked_value::live == 1);
    CHECK(tracked_value::constructed == 3);
    CHECK(tracked_value::destroyed == 2);

    {
        tracked_value removed = buffer.pop_back();

        CHECK(removed.value == 2);
        CHECK(buffer.empty());
        CHECK(tracked_value::live == 1);
        CHECK(tracked_value::constructed == 4);
        CHECK(tracked_value::destroyed == 3);
        CHECK(tracked_value::moved == 2);
    }

    CHECK(buffer.empty());
    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 4);
    CHECK(tracked_value::destroyed == 4);
}

TEST_CASE("null popper destroys the raw-storage slot without creating a return value") {
    tracked_value::live        = 0;
    tracked_value::constructed = 0;
    tracked_value::destroyed   = 0;
    tracked_value::copied      = 0;
    tracked_value::moved       = 0;

    ztl::ring_buffer<tracked_value, 2, ztl::null_popper<tracked_value>> buffer;

    buffer.emplace_back(1);
    buffer.emplace_back(2);

    static_assert(std::is_same<decltype(buffer.pop_front()), void>::value,
                  "null_popper<T> should make pop_front() return void");
    static_assert(std::is_same<decltype(buffer.pop_back()), void>::value,
                  "null_popper<T> should make pop_back() return void");

    buffer.pop_front();

    CHECK(buffer.size() == 1U);
    CHECK(buffer.front().value == 2);
    CHECK(tracked_value::live == 1);
    CHECK(tracked_value::constructed == 2);
    CHECK(tracked_value::destroyed == 1);
    CHECK(tracked_value::moved == 0);

    buffer.clear();

    CHECK(buffer.empty());
    CHECK(tracked_value::live == 0);
    CHECK(tracked_value::constructed == 2);
    CHECK(tracked_value::destroyed == 2);
}

TEST_CASE("default popper returns the removed value") {
    ztl::ring_buffer<int, 2> buffer;

    buffer.push_back(4);
    buffer.push_back(5);

    static_assert(std::is_same<decltype(buffer.pop_front()), int>::value,
                  "default_popper<T> should make pop_front() return T");
    static_assert(std::is_same<decltype(buffer.pop_back()), int>::value,
                  "default_popper<T> should make pop_back() return T");
    CHECK(buffer.pop_front() == 4);
    CHECK(buffer.pop_back() == 5);
    CHECK(buffer.empty());
}

TEST_CASE("null popper makes pop_front return void") {
    ztl::ring_buffer<int, 2, ztl::null_popper<int>> buffer;

    buffer.push_back(4);
    buffer.push_back(5);

    static_assert(std::is_same<decltype(buffer.pop_front()), void>::value,
                  "null_popper<T> should make pop_front() return void");
    static_assert(std::is_same<decltype(buffer.pop_back()), void>::value,
                  "null_popper<T> should make pop_back() return void");
    buffer.pop_front();

    CHECK(buffer.size() == 1U);
    CHECK(buffer.front() == 5);
}

TEST_CASE("copy popper returns the removed value with raw storage") {
    ztl::ring_buffer<int, 2, ztl::copy_popper<int>> buffer{ztl::copy_popper<int>{42}};

    buffer.push_back(4);
    buffer.push_back(5);

    CHECK(buffer.pop_front() == 4);
    CHECK(buffer.pop_front() == 5);
    CHECK(buffer.empty());

    buffer.push_back(7);
    CHECK(buffer.front() == 7);
}

TEST_CASE_TEMPLATE("iterator exposes mutable random-access behavior", T, int, std::uint16_t, small_value, wide_value) {
    typedef ztl::ring_buffer<T, 5> buffer_type;

    buffer_type buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.push_back(T(4));
    buffer.push_back(T(5));

    CHECK((buffer.pop_front() == T(1)));
    CHECK((buffer.pop_front() == T(2)));
    buffer.push_back(T(6));
    buffer.push_back(T(7));

    static_assert(
        std::is_same<typename buffer_type::iterator::iterator_category, std::random_access_iterator_tag>::value,
        "ring_buffer iterator should be a random-access iterator");
    static_assert(std::is_same<typename buffer_type::iterator::value_type, T>::value,
                  "ring_buffer iterator value_type should be T");
    static_assert(std::is_same<typename buffer_type::iterator::difference_type, std::ptrdiff_t>::value,
                  "ring_buffer iterator difference_type should be ptrdiff_t");

    typename buffer_type::iterator first = buffer.begin();
    typename buffer_type::iterator last  = buffer.end();

    CHECK(last - first == 5);
    CHECK((*(first + 0) == T(3)));
    CHECK((*(first + 1) == T(4)));
    CHECK((*(first + 2) == T(5)));
    CHECK((*(first + 3) == T(6)));
    CHECK((*(first + 4) == T(7)));
    CHECK((*(4 + first) == T(7)));
    CHECK((first[2] == T(5)));

    CHECK(first < last);
    CHECK(first <= last);
    CHECK(last > first);
    CHECK(last >= first);
    CHECK(first + 5 == last);
    CHECK(last - 5 == first);

    *(buffer.begin() + 1) = T(40);
    buffer.begin()[3]     = T(60);

    CHECK((buffer[0] == T(3)));
    CHECK((buffer[1] == T(40)));
    CHECK((buffer[2] == T(5)));
    CHECK((buffer[3] == T(60)));
    CHECK((buffer[4] == T(7)));
}

TEST_CASE_TEMPLATE("iterator pre/post increment and decrement move one logical position", T, int, std::uint16_t,
                   small_value, wide_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type buffer;

    buffer.push_back(T(10));
    buffer.push_back(T(20));
    buffer.push_back(T(30));

    typename buffer_type::iterator it = buffer.begin();

    CHECK((*it == T(10)));
    CHECK((*(it++) == T(10)));
    CHECK((*it == T(20)));
    CHECK((*(++it) == T(30)));
    CHECK((*--it == T(20)));
    CHECK((*(it--) == T(20)));
    CHECK((*it == T(10)));
}

TEST_CASE_TEMPLATE("const and non-const iterators compare and subtract across constness", T, int, std::uint16_t,
                   small_value, wide_value) {
    typedef ztl::ring_buffer<T, 5> buffer_type;

    buffer_type buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.push_back(T(4));
    buffer.push_back(T(5));

    CHECK((buffer.pop_front() == T(1)));
    CHECK((buffer.pop_front() == T(2)));
    buffer.push_back(T(6));
    buffer.push_back(T(7));

    const buffer_type& const_buffer = buffer;

    typename buffer_type::iterator       mutable_first          = buffer.begin();
    typename buffer_type::const_iterator const_first            = const_buffer.cbegin();
    typename buffer_type::const_iterator converted_from_mutable = mutable_first;

    static_assert(std::is_convertible<typename buffer_type::iterator, typename buffer_type::const_iterator>::value,
                  "iterator should convert to const_iterator");
    static_assert(!std::is_convertible<typename buffer_type::const_iterator, typename buffer_type::iterator>::value,
                  "const_iterator should not convert to iterator");

    CHECK((mutable_first == const_first));
    CHECK((const_first == mutable_first));
    CHECK_FALSE((mutable_first != const_first));
    CHECK_FALSE((const_first != mutable_first));
    CHECK((converted_from_mutable == const_first));

    CHECK((mutable_first + 2 == const_first + 2));
    CHECK((const_first + 2 == mutable_first + 2));
    CHECK((buffer.end() == const_buffer.cend()));
    CHECK((const_buffer.cend() == buffer.end()));

    CHECK((buffer.end() - const_buffer.cbegin() == 5));
    CHECK((const_buffer.cend() - buffer.begin() == 5));
    CHECK((mutable_first - const_first == 0));
    CHECK((const_first - mutable_first == 0));
    CHECK(((buffer.begin() + 4) - const_buffer.cbegin() == 4));
    CHECK(((const_buffer.cbegin() + 4) - buffer.begin() == 4));

    CHECK((buffer.begin() < const_buffer.cend()));
    CHECK((const_buffer.cbegin() < buffer.end()));
    CHECK((buffer.begin() <= const_buffer.cbegin()));
    CHECK((const_buffer.cend() > buffer.begin()));
    CHECK((buffer.end() >= const_buffer.cbegin()));

    CHECK((*const_first == T(3)));
    CHECK((const_first[1] == T(4)));
    CHECK((const_buffer.cend()[-1] == T(7)));
}

TEST_CASE_TEMPLATE("iterators from different buffers are not equal", T, int, std::uint16_t, small_value, wide_value) {
    typedef ztl::ring_buffer<T, 3> buffer_type;

    buffer_type left;
    buffer_type right;

    left.push_back(T(1));
    right.push_back(T(1));

    const buffer_type& const_left  = left;
    const buffer_type& const_right = right;

    CHECK_FALSE((left.begin() == right.begin()));
    CHECK_FALSE((left.end() == right.end()));
    CHECK_FALSE((const_left.cbegin() == right.begin()));
    CHECK_FALSE((left.begin() == const_right.cbegin()));
}

TEST_CASE("iterator operator_arrow works for mutable and const user types") {
    ztl::ring_buffer<small_value, 2> buffer;

    buffer.emplace_back(10);

    CHECK(buffer.begin()->value == 10);
    buffer.begin()->value = 20;
    CHECK(buffer.front().value == 20);

    const ztl::ring_buffer<small_value, 2>& const_buffer = buffer;
    CHECK(const_buffer.cbegin()->value == 20);
}

TEST_CASE_TEMPLATE("iterators work with non-default-constructible raw-storage value types", T, non_default_value,
                   non_assignable_value) {
    typedef ztl::ring_buffer<T, 4> buffer_type;

    buffer_type buffer;

    buffer.emplace_back(1);
    buffer.emplace_back(2);
    buffer.emplace_back(3);

    const buffer_type& const_buffer = buffer;

    typename buffer_type::iterator       mutable_first = buffer.begin();
    typename buffer_type::const_iterator const_first   = const_buffer.cbegin();

    CHECK((mutable_first == const_first));
    CHECK((const_first == mutable_first));
    CHECK((buffer.end() == const_buffer.cend()));
    CHECK((const_buffer.cend() == buffer.end()));

    CHECK((mutable_first + 2 == const_first + 2));
    CHECK((const_first + 2 == mutable_first + 2));
    CHECK((buffer.end() - const_buffer.cbegin() == 3));
    CHECK((const_buffer.cend() - buffer.begin() == 3));

    CHECK((mutable_first < const_buffer.cend()));
    CHECK((const_first < buffer.end()));
    CHECK((buffer.end() > const_first));
    CHECK((const_buffer.cend() >= mutable_first));

    CHECK((*mutable_first == T(1)));
    CHECK((mutable_first[1] == T(2)));
    CHECK((const_first[2] == T(3)));
    CHECK((std::find(const_buffer.cbegin(), const_buffer.cend(), T(2)) == const_buffer.cbegin() + 1));
}

TEST_CASE("const and non-const iterators work with move-only non-default values") {
    typedef ztl::ring_buffer<move_only_value, 3> buffer_type;

    buffer_type buffer;

    buffer.emplace_back(1);
    buffer.emplace_back(2);
    buffer.emplace_back(3);

    const buffer_type& const_buffer = buffer;

    typename buffer_type::iterator       mutable_first = buffer.begin();
    typename buffer_type::const_iterator const_first   = const_buffer.cbegin();

    CHECK((mutable_first == const_first));
    CHECK((const_first == mutable_first));
    CHECK((buffer.end() == const_buffer.cend()));
    CHECK((const_buffer.cend() - mutable_first == 3));
    CHECK((buffer.end() - const_first == 3));

    CHECK((*mutable_first == move_only_value(1)));
    CHECK((mutable_first[1] == move_only_value(2)));
    CHECK((const_first[2] == move_only_value(3)));
}

TEST_CASE_TEMPLATE("standard algorithms work with iterator ranges", T, int, std::uint16_t, small_value, wide_value) {
    typedef ztl::ring_buffer<T, 5> buffer_type;

    buffer_type buffer;

    buffer.push_back(T(1));
    buffer.push_back(T(2));
    buffer.push_back(T(3));
    buffer.push_back(T(4));
    buffer.push_back(T(5));

    CHECK(std::find(buffer.begin(), buffer.end(), T(3)) == buffer.begin() + 2);
    CHECK(std::equal(buffer.begin(), buffer.end(), std::vector<T>{T(1), T(2), T(3), T(4), T(5)}.begin()));

    std::reverse_iterator<typename buffer_type::iterator> rfirst(buffer.end());
    std::reverse_iterator<typename buffer_type::iterator> rlast(buffer.begin());
    const std::vector<T>                                  reversed(rfirst, rlast);

    CHECK(reversed.size() == 5U);
    CHECK((reversed[0] == T(5)));
    CHECK((reversed[1] == T(4)));
    CHECK((reversed[2] == T(3)));
    CHECK((reversed[3] == T(2)));
    CHECK((reversed[4] == T(1)));
}

TEST_CASE("numeric algorithms work with mutable iterator ranges") {
    ztl::ring_buffer<int, 5> buffer;

    buffer.push_back(1);
    buffer.push_back(2);
    buffer.push_back(3);
    buffer.push_back(4);
    buffer.push_back(5);

    CHECK(std::accumulate(buffer.begin(), buffer.end(), 0) == 15);
}
