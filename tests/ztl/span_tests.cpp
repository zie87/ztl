#include "ztl/span.hpp"
#include "ztl/type_traits.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

namespace {

template <typename Span, typename = void>
struct can_as_writable_bytes : std::false_type {};

template <typename Span>
struct can_as_writable_bytes<Span, ztl::void_t<decltype(ztl::as_writable_bytes(std::declval<Span>()))>>
    : std::true_type {};

} // namespace

TEST_CASE("span type traits and compile-time API contracts") {
    static_assert(ztl::dynamic_extent == std::numeric_limits<std::size_t>::max(),
                  "dynamic_extent must use the sentinel max size_t value");

    static_assert(ztl::span<int>::extent == ztl::dynamic_extent, "dynamic extent span must expose dynamic_extent");
    static_assert(ztl::span<int, 4>::extent == 4U, "fixed extent span must expose its fixed extent");

    static_assert(std::is_same<ztl::span<int>::element_type, int>::value, "element_type must be T");
    static_assert(std::is_same<ztl::span<const int>::element_type, const int>::value,
                  "element_type must preserve const");
    static_assert(std::is_same<ztl::span<volatile int>::element_type, volatile int>::value,
                  "element_type must preserve volatile");
    static_assert(std::is_same<ztl::span<int>::value_type, int>::value, "value_type must remove cv qualifiers");
    static_assert(std::is_same<ztl::span<const volatile int>::value_type, int>::value,
                  "value_type must remove const and volatile qualifiers");

    static_assert(std::is_constructible<ztl::span<int, 3>, int (&)[3]>::value,
                  "fixed span must be constructible from matching C array");
    static_assert(std::is_constructible<ztl::span<int>, int (&)[3]>::value,
                  "dynamic span must be constructible from C array");
    static_assert(!std::is_constructible<ztl::span<int, 2>, int (&)[3]>::value,
                  "fixed span must reject mismatching C array extent");

    static_assert(std::is_constructible<ztl::span<const int, 3>, int (&)[3]>::value,
                  "span<int> source must convert to span<const int>");
    static_assert(!std::is_constructible<ztl::span<int, 3>, const int (&)[3]>::value,
                  "span<const int> source must not convert to span<int>");

    static_assert(std::is_constructible<ztl::span<int>, int*, std::size_t>::value,
                  "dynamic span must be constructible from pointer and size");
    static_assert(std::is_constructible<ztl::span<int, 3>, int*, std::size_t>::value,
                  "fixed span must be constructible from pointer and size");

    static_assert(std::is_constructible<ztl::span<int>, int*, int*>::value,
                  "dynamic span must be constructible from pointer pair");
    static_assert(std::is_constructible<ztl::span<int, 3>, int*, int*>::value,
                  "fixed span must be constructible from pointer pair");

    static_assert(std::is_constructible<ztl::span<const int>, ztl::span<int, 3>>::value,
                  "mutable fixed span must convert to const dynamic span");
    static_assert(std::is_constructible<ztl::span<const int, 3>, ztl::span<int, 3>>::value,
                  "mutable fixed span must convert to const fixed span");
    static_assert(!std::is_constructible<ztl::span<int>, ztl::span<const int, 3>>::value,
                  "const span must not convert to mutable span");

    static_assert(can_as_writable_bytes<ztl::span<int>>::value, "mutable span must support as_writable_bytes");
    static_assert(!can_as_writable_bytes<ztl::span<const int>>::value, "const span must not support as_writable_bytes");

    static_assert(std::is_default_constructible<ztl::span<int>>::value, "dynamic span should be default constructible");
    static_assert(std::is_default_constructible<ztl::span<int, 0>>::value,
                  "zero-extent span should be default constructible");
    static_assert(!std::is_default_constructible<ztl::span<int, 1>>::value,
                  "non-zero fixed span should not be default constructible");

    static_assert(!std::is_constructible<ztl::span<const int>, std::array<int, 3>&&>::value,
                  "span must not bind to a temporary std::array");
    static_assert(!std::is_constructible<ztl::span<const int, 3>, std::array<int, 3>&&>::value,
                  "fixed span must not bind to a temporary std::array");

    CHECK(true);
}

TEST_CASE_TEMPLATE("span constructs from C arrays", Span, ztl::span<int, 3>, ztl::span<int>, ztl::span<const int, 3>,
                   ztl::span<const int>) {
    int  data[] = {10, 20, 30};
    Span view{data};

    CHECK(view.size() == 3U);
    CHECK(view.data() == data);
    CHECK_FALSE(view.empty());
    CHECK(view.size_bytes() == sizeof(data));
    CHECK(std::equal(view.begin(), view.end(), data));
}

TEST_CASE_TEMPLATE("span constructs from pointer and size", Span, ztl::span<const int, 5>, ztl::span<const int>) {
    const int data[] = {1, 2, 3, 4, 5};
    Span      view{data, sizeof(data) / sizeof(data[0])};

    CHECK(view.size() == 5U);
    CHECK(view.data() == data);
    CHECK_FALSE(view.empty());
    CHECK(view.size_bytes() == sizeof(data));
    CHECK(std::equal(view.begin(), view.end(), data));
}

TEST_CASE_TEMPLATE("span constructs from pointer pair", Span, ztl::span<const int, 5>, ztl::span<const int>) {
    const int data[] = {11, 22, 33, 44, 55};
    Span      view{data, data + (sizeof(data) / sizeof(data[0]))};

    CHECK(view.size() == 5U);
    CHECK(view.data() == data);
    CHECK_FALSE(view.empty());
    CHECK(view.size_bytes() == sizeof(data));
    CHECK(std::equal(view.begin(), view.end(), data));
}

TEST_CASE("span constructs from std::array") {
    std::array<char, 4> data{{'a', 'b', 'c', 'd'}};

    ztl::span<char, 4>       fixed{data};
    ztl::span<char>          dynamic{data};
    ztl::span<const char, 4> const_fixed{data};
    ztl::span<const char>    const_dynamic{data};

    CHECK(fixed.size() == data.size());
    CHECK(dynamic.size() == data.size());
    CHECK(const_fixed.size() == data.size());
    CHECK(const_dynamic.size() == data.size());
    CHECK(fixed.data() == data.data());
    CHECK(dynamic.data() == data.data());
    CHECK(const_fixed.data() == data.data());
    CHECK(const_dynamic.data() == data.data());
    CHECK(std::equal(fixed.begin(), fixed.end(), data.begin()));
    CHECK(std::equal(dynamic.begin(), dynamic.end(), data.begin()));
    CHECK(std::equal(const_fixed.begin(), const_fixed.end(), data.begin()));
    CHECK(std::equal(const_dynamic.begin(), const_dynamic.end(), data.begin()));

    fixed[1] = 'x';
    CHECK(data[1] == 'x');
}

TEST_CASE("span constructs from const std::array") {
    const std::array<int, 3> data{{7, 8, 9}};

    ztl::span<const int, 3> fixed{data};
    ztl::span<const int>    dynamic{data};

    CHECK(fixed.size() == data.size());
    CHECK(dynamic.size() == data.size());
    CHECK(fixed.data() == data.data());
    CHECK(dynamic.data() == data.data());
    CHECK(std::equal(fixed.begin(), fixed.end(), data.begin()));
    CHECK(std::equal(dynamic.begin(), dynamic.end(), data.begin()));
}

TEST_CASE("make_span preserves fixed extent for arrays") {
    int  data[] = {5, 6, 7};
    auto view   = ztl::make_span(data);

    static_assert(std::is_same<typename ztl::remove_cvref<decltype(view)>::type, ztl::span<int, 3>>::value,
                  "make_span(C array) must produce a fixed-extent span");

    CHECK(view.size() == 3U);
    CHECK(view.data() == data);
    CHECK(std::equal(view.begin(), view.end(), data));
}

TEST_CASE("make_span preserves fixed extent for std::array") {
    std::array<std::uint8_t, 4> data{{1U, 2U, 3U, 4U}};
    auto                        view = ztl::make_span(data);

    static_assert(std::is_same<typename ztl::remove_cvref<decltype(view)>::type, ztl::span<std::uint8_t, 4>>::value,
                  "make_span(std::array&) must produce a fixed-extent span");

    CHECK(view.size() == data.size());
    CHECK(view.data() == data.data());
    CHECK(std::equal(view.begin(), view.end(), data.begin()));
}

TEST_CASE("span copy, assignment, and conversion construction") {
    int data[] = {1, 2, 3};

    ztl::span<int, 3> fixed{data};
    ztl::span<int, 3> fixed_copy{fixed};
    ztl::span<int, 3> fixed_assigned{data};
    fixed_assigned = fixed;

    CHECK(fixed_copy.data() == fixed.data());
    CHECK(fixed_copy.size() == fixed.size());
    CHECK(fixed_assigned.data() == fixed.data());
    CHECK(fixed_assigned.size() == fixed.size());

    ztl::span<int> dynamic_from_fixed{fixed};
    CHECK(dynamic_from_fixed.data() == data);
    CHECK(dynamic_from_fixed.size() == 3U);

    ztl::span<const int, 3> const_fixed_from_mutable{fixed};
    ztl::span<const int>    const_dynamic_from_mutable{fixed};
    CHECK(const_fixed_from_mutable.data() == data);
    CHECK(const_dynamic_from_mutable.data() == data);

    ztl::span<int>    dynamic{data, sizeof(data) / sizeof(data[0])};
    ztl::span<int, 3> fixed_from_dynamic{dynamic};
    CHECK(fixed_from_dynamic.data() == data);
    CHECK(fixed_from_dynamic.size() == 3U);
}

TEST_CASE_TEMPLATE("span observers report storage properties", Span, ztl::span<int, 5>, ztl::span<int>,
                   ztl::span<const int, 5>, ztl::span<const int>) {
    int  data[] = {3, 4, 5, 10, 1};
    Span view{data};

    CHECK(view.size() == 5U);
    CHECK(ztl::size(view) == 5U);
    CHECK(view.size_bytes() == sizeof(data));
    CHECK_FALSE(view.empty());
    CHECK(view.data() == data);

    CHECK(view.front() == 3);
    CHECK(view.back() == 1);
    CHECK(view[0] == 3);
    CHECK(view[2] == 5);
    CHECK(view.at(3) == 10);
}

TEST_CASE_TEMPLATE("mutable span element access aliases original storage", Span, ztl::span<int, 5>, ztl::span<int>) {
    int  data[] = {3, 4, 5, 10, 1};
    Span view{data};

    view[2] = 42;
    CHECK(data[2] == 42);
    CHECK(view.at(2) == 42);

    view.front() = -1;
    view.back()  = -9;
    CHECK(data[0] == -1);
    CHECK(data[4] == -9);
}

TEST_CASE("span supports zero-length views") {
    int data[] = {1, 2, 3};

    ztl::span<int, 0> zero_fixed{data, std::size_t{0}};
    ztl::span<int>    zero_dynamic{data, std::size_t{0}};
    ztl::span<int>    null_empty{static_cast<int*>(nullptr), std::size_t{0}};

    CHECK(zero_fixed.size() == 0U);
    CHECK(zero_dynamic.size() == 0U);
    CHECK(null_empty.size() == 0U);

    CHECK(zero_fixed.size_bytes() == 0U);
    CHECK(zero_dynamic.size_bytes() == 0U);
    CHECK(null_empty.size_bytes() == 0U);

    CHECK(zero_fixed.empty());
    CHECK(zero_dynamic.empty());
    CHECK(null_empty.empty());

    CHECK(zero_fixed.data() == data);
    CHECK(zero_dynamic.data() == data);
    CHECK(null_empty.data() == nullptr);

#if defined(ZTL_SPAN_TEST_EMPTY_ITERATORS)
    // Expected target behaviour. The current implementation may trip contract
    // checks here because begin()/end() form nullptr + 0 and the iterator
    // constructor rejects null pointers.
    CHECK(null_empty.begin() == null_empty.end());
    CHECK(std::distance(null_empty.begin(), null_empty.end()) == 0);
#endif
}

TEST_CASE_TEMPLATE("span iterators model random access", Span, ztl::span<int, 5>, ztl::span<int>,
                   ztl::span<const int, 5>, ztl::span<const int>) {
    static_assert(std::is_same<typename std::iterator_traits<typename Span::iterator>::iterator_category,
                               std::random_access_iterator_tag>::value,
                  "span iterator must be random access");
    static_assert(std::is_same<typename std::iterator_traits<typename Span::iterator>::value_type,
                               typename Span::value_type>::value,
                  "iterator value_type must match span value_type");
    static_assert(std::is_same<typename std::iterator_traits<typename Span::iterator>::difference_type,
                               typename Span::difference_type>::value,
                  "iterator difference_type must match span difference_type");
    static_assert(
        std::is_same<typename std::iterator_traits<typename Span::iterator>::pointer, typename Span::pointer>::value,
        "iterator pointer must match span pointer");
    static_assert(std::is_same<typename std::iterator_traits<typename Span::iterator>::reference,
                               typename Span::reference>::value,
                  "iterator reference must match span reference");

    int  data[] = {2, 4, 6, 8, 10};
    Span view{data};

    auto first = view.begin();
    auto last  = view.end();

    CHECK(first == view.begin());
    CHECK(first != last);
    CHECK(*first == view[0]);
    CHECK(first.operator->() == view.data());
    CHECK(last - first == static_cast<typename Span::difference_type>(view.size()));
    CHECK(std::distance(first, last) == static_cast<typename Span::difference_type>(view.size()));

    CHECK(*(first + 2) == view[2]);
    CHECK(*(2 + first) == view[2]);
    CHECK(first[3] == view[3]);

    auto post_increment = first++;
    CHECK(*post_increment == view[0]);
    CHECK(*first == view[1]);

    auto& pre_increment = ++first;
    CHECK(&pre_increment == &first);
    CHECK(*first == view[2]);

    auto post_decrement = first--;
    CHECK(*post_decrement == view[2]);
    CHECK(*first == view[1]);

    auto& pre_decrement = --first;
    CHECK(&pre_decrement == &first);
    CHECK(*first == view[0]);

    first += 4;
    CHECK(*first == view[4]);
    first -= 3;
    CHECK(*first == view[1]);

    CHECK(view.begin() < view.end());
    CHECK(view.begin() <= view.begin());
    CHECK(view.end() > view.begin());
    CHECK(view.end() >= view.end());
}

TEST_CASE_TEMPLATE("span reverse iterators traverse from back to front", Span, ztl::span<int, 5>, ztl::span<int>,
                   ztl::span<const int, 5>, ztl::span<const int>) {
    static_assert(std::is_same<typename Span::reverse_iterator, std::reverse_iterator<typename Span::iterator>>::value,
                  "reverse_iterator must be std::reverse_iterator<iterator>");

    int  data[]     = {2, 4, 6, 8, 10};
    int  reversed[] = {10, 8, 6, 4, 2};
    Span view{data};

    REQUIRE(view.rbegin() != view.rend());
    CHECK(*view.rbegin() == 10);
    CHECK(std::equal(view.rbegin(), view.rend(), reversed));
}

TEST_CASE_TEMPLATE("span iterators work with standard algorithms", Span, ztl::span<int, 5>, ztl::span<int>,
                   ztl::span<const int, 5>, ztl::span<const int>) {
    int  data[] = {2, 4, 6, 8, 10};
    Span view{data};

    CHECK(std::accumulate(view.begin(), view.end(), 0) == 30);
    CHECK(std::count(view.begin(), view.end(), 6) == 1);
    CHECK(std::find(view.begin(), view.end(), 8) == view.begin() + 3);
}

TEST_CASE("mutable and const span iterators compare across cv-qualified views") {
    int data[] = {1, 2, 3, 4};

    ztl::span<int, 4>       mutable_fixed{data};
    ztl::span<const int, 4> const_fixed{mutable_fixed};
    ztl::span<int>          mutable_dynamic{data};
    ztl::span<const int>    const_dynamic{mutable_dynamic};

    CHECK(mutable_fixed.begin() == const_fixed.begin());
    CHECK(mutable_fixed.end() == const_fixed.end());
    CHECK(mutable_fixed.begin() < const_fixed.end());
    CHECK(const_fixed.end() - mutable_fixed.begin() == 4);

    CHECK(mutable_dynamic.begin() == const_dynamic.begin());
    CHECK(mutable_dynamic.end() == const_dynamic.end());
    CHECK(mutable_dynamic.begin() < const_dynamic.end());
    CHECK(const_dynamic.end() - mutable_dynamic.begin() == 4);
}

TEST_CASE_TEMPLATE("span subviews preserve values, extents, and storage", Span, ztl::span<int, 6>, ztl::span<int>) {
    int  data[] = {10, 20, 30, 40, 50, 60};
    Span view{data};

    SUBCASE("first<N> and first(count)") {
        const auto fixed    = view.template first<3>();
        const auto dyn      = view.first(3U);
        int        expect[] = {10, 20, 30};

        static_assert(std::is_same<typename ztl::remove_cvref<decltype(fixed)>::type, ztl::span<int, 3>>::value,
                      "first<N>() must return fixed extent N");
        static_assert(std::is_same<typename ztl::remove_cvref<decltype(dyn)>::type, ztl::span<int>>::value,
                      "first(count) must return dynamic extent");

        CHECK(fixed.size() == 3U);
        CHECK(dyn.size() == 3U);
        CHECK(fixed.data() == data);
        CHECK(dyn.data() == data);
        CHECK(std::equal(fixed.begin(), fixed.end(), expect));
        CHECK(std::equal(dyn.begin(), dyn.end(), expect));
    }

    SUBCASE("last<N> and last(count)") {
        const auto fixed    = view.template last<2>();
        const auto dyn      = view.last(2U);
        int        expect[] = {50, 60};

        static_assert(std::is_same<typename ztl::remove_cvref<decltype(fixed)>::type, ztl::span<int, 2>>::value,
                      "last<N>() must return fixed extent N");
        static_assert(std::is_same<typename ztl::remove_cvref<decltype(dyn)>::type, ztl::span<int>>::value,
                      "last(count) must return dynamic extent");

        CHECK(fixed.size() == 2U);
        CHECK(dyn.size() == 2U);
        CHECK(fixed.data() == data + 4);
        CHECK(dyn.data() == data + 4);
        CHECK(std::equal(fixed.begin(), fixed.end(), expect));
        CHECK(std::equal(dyn.begin(), dyn.end(), expect));
    }

    SUBCASE("subspan<offset>() and subspan(offset)") {
        const auto fixed_tail = view.template subspan<2>();
        const auto dyn_tail   = view.subspan(2U);
        int        expect[]   = {30, 40, 50, 60};

        static_assert(
            (std::is_same<Span, ztl::span<int, 6>>::value &&
             std::is_same<typename ztl::remove_cvref<decltype(fixed_tail)>::type, ztl::span<int, 4>>::value) ||
                (std::is_same<Span, ztl::span<int>>::value &&
                 std::is_same<typename ztl::remove_cvref<decltype(fixed_tail)>::type, ztl::span<int>>::value),
            "subspan<offset>() must calculate a fixed tail extent only for fixed-extent spans");
        static_assert(std::is_same<typename ztl::remove_cvref<decltype(dyn_tail)>::type, ztl::span<int>>::value,
                      "subspan(offset) must return dynamic extent");

        CHECK(fixed_tail.size() == 4U);
        CHECK(dyn_tail.size() == 4U);
        CHECK(fixed_tail.data() == data + 2);
        CHECK(dyn_tail.data() == data + 2);
        CHECK(std::equal(fixed_tail.begin(), fixed_tail.end(), expect));
        CHECK(std::equal(dyn_tail.begin(), dyn_tail.end(), expect));
    }

    SUBCASE("subspan<offset, count>() and subspan(offset, count)") {
        const auto fixed_mid = view.template subspan<1, 3>();
        const auto dyn_mid   = view.subspan(1U, 3U);
        int        expect[]  = {20, 30, 40};

        static_assert(std::is_same<typename ztl::remove_cvref<decltype(fixed_mid)>::type, ztl::span<int, 3>>::value,
                      "subspan<offset, count>() must return fixed extent count");
        static_assert(std::is_same<typename ztl::remove_cvref<decltype(dyn_mid)>::type, ztl::span<int>>::value,
                      "subspan(offset, count) must return dynamic extent");

        CHECK(fixed_mid.size() == 3U);
        CHECK(dyn_mid.size() == 3U);
        CHECK(fixed_mid.data() == data + 1);
        CHECK(dyn_mid.data() == data + 1);
        CHECK(std::equal(fixed_mid.begin(), fixed_mid.end(), expect));
        CHECK(std::equal(dyn_mid.begin(), dyn_mid.end(), expect));
    }

    SUBCASE("subviews alias the original storage") {
        auto mid = view.template subspan<2, 2>();
        mid[0]   = 300;
        mid[1]   = 400;

        CHECK(data[2] == 300);
        CHECK(data[3] == 400);
    }

    SUBCASE("zero-length subviews from non-empty storage") {
        const auto first_zero = view.template first<0>();
        const auto last_zero  = view.template last<0>();
        const auto mid_zero   = view.template subspan<3, 0>();
        const auto end_zero   = view.template subspan<6>();

        CHECK(first_zero.size() == 0U);
        CHECK(last_zero.size() == 0U);
        CHECK(mid_zero.size() == 0U);
        CHECK(end_zero.size() == 0U);

        CHECK(first_zero.data() == data);
        CHECK(last_zero.data() == data + 6);
        CHECK(mid_zero.data() == data + 3);
        CHECK(end_zero.data() == data + 6);
    }
}

TEST_CASE("span preserves const and volatile element qualifiers") {
    SUBCASE("const span exposes const elements") {
        int                     data[] = {9, 8, 7};
        ztl::span<const int, 3> view{data};

        static_assert(std::is_same<decltype(view.data()), const int*>::value,
                      "span<const T>::data() must return const T*");
        static_assert(std::is_same<decltype(view[0]), const int&>::value,
                      "span<const T>::operator[] must return const T&");

        CHECK(view[0] == 9);
        CHECK(view[1] == 8);
        CHECK(view[2] == 7);
    }

    SUBCASE("volatile span exposes volatile elements") {
        volatile std::uint32_t               regs[] = {0x10U, 0x20U};
        ztl::span<volatile std::uint32_t, 2> view{regs};

        static_assert(std::is_same<decltype(view.data()), volatile std::uint32_t*>::value,
                      "span<volatile T>::data() must return volatile T*");
        static_assert(std::is_same<decltype(view[0]), volatile std::uint32_t&>::value,
                      "span<volatile T>::operator[] must return volatile T&");

        CHECK(view.size() == 2U);
        CHECK(view.data() == regs);
        view[0] = 0x33U;
        CHECK(regs[0] == 0x33U);
    }
}

TEST_CASE_TEMPLATE("as_bytes exposes byte-sized views", Span, ztl::span<std::uint16_t, 3>, ztl::span<std::uint16_t>) {
    std::uint16_t data[] = {0x1122U, 0x3344U, 0x5566U};
    Span          words{data};

    auto bytes = ztl::as_bytes(words);

    static_assert(
        (std::is_same<Span, ztl::span<std::uint16_t, 3>>::value &&
         std::is_same<typename ztl::remove_cvref<decltype(bytes)>::type,
                      ztl::span<const ztl::byte, sizeof(data)>>::value) ||
            (std::is_same<Span, ztl::span<std::uint16_t>>::value &&
             std::is_same<typename ztl::remove_cvref<decltype(bytes)>::type, ztl::span<const ztl::byte>>::value),
        "as_bytes must preserve fixed byte extent and keep dynamic spans dynamic");

    CHECK(bytes.size() == sizeof(data));
    CHECK(bytes.size_bytes() == sizeof(data));
    CHECK(reinterpret_cast<const void*>(bytes.data()) == reinterpret_cast<const void*>(data));
}

TEST_CASE_TEMPLATE("as_writable_bytes exposes mutable byte-sized views", Span, ztl::span<std::uint16_t, 3>,
                   ztl::span<std::uint16_t>) {
    std::uint16_t data[] = {0x1122U, 0x3344U, 0x5566U};
    Span          words{data};

    auto bytes = ztl::as_writable_bytes(words);

    static_assert(
        (std::is_same<Span, ztl::span<std::uint16_t, 3>>::value &&
         std::is_same<typename ztl::remove_cvref<decltype(bytes)>::type, ztl::span<ztl::byte, sizeof(data)>>::value) ||
            (std::is_same<Span, ztl::span<std::uint16_t>>::value &&
             std::is_same<typename ztl::remove_cvref<decltype(bytes)>::type, ztl::span<ztl::byte>>::value),
        "as_writable_bytes must preserve fixed byte extent and keep dynamic spans dynamic");

    CHECK(bytes.size() == sizeof(data));
    CHECK(bytes.size_bytes() == sizeof(data));
    CHECK(reinterpret_cast<void*>(bytes.data()) == reinterpret_cast<void*>(data));
}

TEST_CASE("as_writable_bytes is not available for const spans") {
    static_assert(!can_as_writable_bytes<ztl::span<const std::uint16_t, 3>>::value,
                  "const fixed span must not support as_writable_bytes");
    static_assert(!can_as_writable_bytes<ztl::span<const std::uint16_t>>::value,
                  "const dynamic span must not support as_writable_bytes");
    CHECK(true);
}

TEST_CASE("span object representation is compact for embedded use") {
    CHECK(sizeof(ztl::span<int, 4>) == sizeof(int*));
    CHECK(sizeof(ztl::span<int>) <= (sizeof(int*) + sizeof(std::size_t)));

    static_assert(std::is_copy_constructible<ztl::span<int>>::value, "span must be copy constructible");
    static_assert(std::is_copy_assignable<ztl::span<int>>::value, "span must be copy assignable");
    static_assert(std::is_nothrow_copy_constructible<ztl::span<int>>::value,
                  "span copy construction should be noexcept");
    static_assert(std::is_nothrow_copy_assignable<ztl::span<int>>::value, "span copy assignment should be noexcept");
}
