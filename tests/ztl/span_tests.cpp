// Unit tests for ztl::span using doctest.
//
// This file intentionally tests the expected span behaviour, not only the
// behaviour of the current implementation. Some tests may expose current
// defects and fail to compile or fail at runtime until span.hpp is fixed.

#include "ztl/span.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

namespace
{

template<typename T, std::size_t N>
constexpr std::size_t array_size(T const (&)[N]) noexcept
{
    return N;
}

template<typename Span, typename T, std::size_t N>
void check_span_content(Span const& s, T const (&expected)[N])
{
    REQUIRE(s.size() == N);
    CHECK(std::equal(s.begin(), s.end(), expected));
}

template<typename T>
struct remove_cv_ref
{
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

template<typename...>
using void_t = void;

template<typename Span, typename = void>
struct can_as_writable_bytes : std::false_type
{
};

template<typename Span>
struct can_as_writable_bytes<Span, void_t<decltype(ztl::as_writable_bytes(std::declval<Span>()))>>
: std::true_type
{
};

} // namespace

TEST_CASE("span type traits and compile-time API contracts")
{
    static_assert(ztl::dynamic_extent == std::numeric_limits<std::size_t>::max(),
                  "dynamic_extent must use the sentinel max size_t value");

    static_assert(ztl::span<int>::extent == ztl::dynamic_extent,
                  "dynamic extent span must expose dynamic_extent");
    static_assert(ztl::span<int, 4>::extent == 4U,
                  "fixed extent span must expose its fixed extent");

    static_assert(std::is_same<ztl::span<int>::element_type, int>::value, "element_type must be T");
    static_assert(std::is_same<ztl::span<const int>::element_type, const int>::value,
                  "element_type must preserve const");
    static_assert(std::is_same<ztl::span<volatile int>::element_type, volatile int>::value,
                  "element_type must preserve volatile");
    static_assert(std::is_same<ztl::span<int>::value_type, int>::value,
                  "value_type must remove cv qualifiers");
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

    static_assert(can_as_writable_bytes<ztl::span<int>>::value,
                  "mutable span must support as_writable_bytes");
    static_assert(!can_as_writable_bytes<ztl::span<const int>>::value,
                  "const span must not support as_writable_bytes");

#if defined(ZTL_SPAN_TEST_EXPECTED_STD_SPAN_COMPAT)
    // These are recommended target behaviours. Enable this block once the span
    // is hardened to std::span/GSL-like expectations.
    static_assert(std::is_default_constructible<ztl::span<int>>::value,
                  "dynamic span should be default constructible");
    static_assert(std::is_default_constructible<ztl::span<int, 0>>::value,
                  "zero-extent span should be default constructible");
    static_assert(!std::is_default_constructible<ztl::span<int, 1>>::value,
                  "non-zero fixed span should not be default constructible");

    static_assert(!std::is_constructible<ztl::span<const int>, std::array<int, 3>&&>::value,
                  "span must not bind to a temporary std::array");
    static_assert(!std::is_constructible<ztl::span<const int, 3>, std::array<int, 3>&&>::value,
                  "fixed span must not bind to a temporary std::array");
#endif

    CHECK(true);
}

TEST_CASE("span construction from arrays, pointers, std::array, and make_span")
{
    SUBCASE("construct fixed and dynamic spans from a C array")
    {
        int data[] = {10, 20, 30, 40};

        ztl::span<int, 4> fixed{data};
        ztl::span<int>    dynamic{data};

        CHECK(fixed.size() == array_size(data));
        CHECK(dynamic.size() == array_size(data));
        CHECK(fixed.data() == data);
        CHECK(dynamic.data() == data);
        CHECK_FALSE(fixed.empty());
        CHECK_FALSE(dynamic.empty());
        CHECK(fixed.size_bytes() == sizeof(data));
        CHECK(dynamic.size_bytes() == sizeof(data));
        check_span_content(fixed, data);
        check_span_content(dynamic, data);
    }

    SUBCASE("construct fixed and dynamic spans from pointer and size")
    {
        const int data[] = {1, 2, 3, 4, 5};

        ztl::span<const int, 5> fixed{data, array_size(data)};
        ztl::span<const int>    dynamic{data, array_size(data)};

        CHECK(fixed.size() == array_size(data));
        CHECK(dynamic.size() == array_size(data));
        CHECK(fixed.data() == data);
        CHECK(dynamic.data() == data);
        check_span_content(fixed, data);
        check_span_content(dynamic, data);
    }

    SUBCASE("construct fixed and dynamic spans from pointer pair")
    {
        const std::uint16_t data[] = {11U, 22U, 33U, 44U, 55U, 66U};

        ztl::span<const std::uint16_t, 6> fixed{data, data + array_size(data)};
        ztl::span<const std::uint16_t>    dynamic{data, data + array_size(data)};

        CHECK(fixed.size() == array_size(data));
        CHECK(dynamic.size() == array_size(data));
        CHECK(fixed.data() == data);
        CHECK(dynamic.data() == data);
        check_span_content(fixed, data);
        check_span_content(dynamic, data);
    }

    SUBCASE("construct spans from std::array")
    {
        std::array<char, 4> data{
            {'a', 'b', 'c', 'd'}
        };

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

        fixed[1] = 'x';
        CHECK(data[1] == 'x');
    }

    SUBCASE("construct spans from const std::array")
    {
        const std::array<int, 3> data{
            {7, 8, 9}
        };

        ztl::span<const int, 3> fixed{data};
        ztl::span<const int>    dynamic{data};

        CHECK(fixed.size() == data.size());
        CHECK(dynamic.size() == data.size());
        CHECK(fixed.data() == data.data());
        CHECK(dynamic.data() == data.data());
    }

    SUBCASE("make_span preserves fixed extent for arrays")
    {
        int  data[] = {5, 6, 7};
        auto view   = ztl::make_span(data);

        using view_type = typename remove_cv_ref<decltype(view)>::type;
        static_assert(std::is_same<view_type, ztl::span<int, 3>>::value,
                      "make_span(C array) must produce a fixed-extent span");

        CHECK(view.size() == array_size(data));
        CHECK(view.data() == data);
        check_span_content(view, data);
    }

    SUBCASE("make_span preserves fixed extent for std::array")
    {
        std::array<std::uint8_t, 4> data{
            {1U, 2U, 3U, 4U}
        };
        auto view = ztl::make_span(data);

        using view_type = typename remove_cv_ref<decltype(view)>::type;
        static_assert(std::is_same<view_type, ztl::span<std::uint8_t, 4>>::value,
                      "make_span(std::array&) must produce a fixed-extent span");

        CHECK(view.size() == data.size());
        CHECK(view.data() == data.data());
    }
}

TEST_CASE("span copy, assignment, and conversion construction")
{
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
    CHECK(dynamic_from_fixed.size() == array_size(data));

    ztl::span<const int, 3> const_fixed_from_mutable{fixed};
    ztl::span<const int>    const_dynamic_from_mutable{fixed};
    CHECK(const_fixed_from_mutable.data() == data);
    CHECK(const_dynamic_from_mutable.data() == data);

    ztl::span<int>    dynamic{data, array_size(data)};
    ztl::span<int, 3> fixed_from_dynamic{dynamic};
    CHECK(fixed_from_dynamic.data() == data);
    CHECK(fixed_from_dynamic.size() == array_size(data));
}

TEST_CASE("span observers and element access")
{
    int               data[] = {3, 4, 5, 10, 1};
    ztl::span<int, 5> view{data};

    CHECK(view.size() == array_size(data));
    CHECK(view.size_bytes() == sizeof(data));
    CHECK_FALSE(view.empty());
    CHECK(view.data() == data);

    CHECK(view.front() == 3);
    CHECK(view.back() == 1);
    CHECK(view[0] == 3);
    CHECK(view[2] == 5);
    CHECK(view.at(3) == 10);

    view[2] = 42;
    CHECK(data[2] == 42);
    CHECK(view.at(2) == 42);

    view.front() = -1;
    view.back()  = -9;
    CHECK(data[0] == -1);
    CHECK(data[4] == -9);
}

TEST_CASE("span supports zero-length views")
{
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

TEST_CASE("span iterators model random access over non-empty spans")
{
    int               data[] = {2, 4, 6, 8, 10};
    ztl::span<int, 5> view{data};

    auto first = view.begin();
    auto last  = view.end();

    CHECK(*first == 2);
    CHECK(*(first + 2) == 6);
    CHECK(*(2 + first) == 6);
    CHECK(first[3] == 8);
    CHECK(last - first == static_cast<std::ptrdiff_t>(array_size(data)));

    ++first;
    CHECK(*first == 4);
    first++;
    CHECK(*first == 6);
    --first;
    CHECK(*first == 4);
    first--;
    CHECK(*first == 2);

    first += 3;
    CHECK(*first == 8);
    first -= 2;
    CHECK(*first == 4);

    CHECK(view.begin() < view.end());
    CHECK(view.begin() <= view.begin());
    CHECK(view.end() > view.begin());
    CHECK(view.end() >= view.end());

    CHECK(std::distance(view.begin(), view.end()) == static_cast<std::ptrdiff_t>(array_size(data)));
    CHECK(std::accumulate(view.begin(), view.end(), 0) == 30);
}

TEST_CASE("span reverse iterators traverse from back to front")
{
    int data[]     = {1, 2, 3, 4};
    int reversed[] = {4, 3, 2, 1};

    ztl::span<int, 4> view{data};

    REQUIRE(view.rbegin() != view.rend());
    CHECK(*view.rbegin() == 4);
    CHECK(std::equal(view.rbegin(), view.rend(), reversed));
}

TEST_CASE("span subviews with first, last, and subspan")
{
    int               data[] = {10, 20, 30, 40, 50, 60};
    ztl::span<int, 6> view{data};

    SUBCASE("first<N> and first(count)")
    {
        const auto fixed    = view.first<3>();
        const auto dyn      = view.first(3U);
        int        expect[] = {10, 20, 30};

        using fixed_type = typename remove_cv_ref<decltype(fixed)>::type;
        using dyn_type   = typename remove_cv_ref<decltype(dyn)>::type;
        static_assert(std::is_same<fixed_type, ztl::span<int, 3>>::value,
                      "first<N>() must return fixed extent N");
        static_assert(std::is_same<dyn_type, ztl::span<int>>::value,
                      "first(count) must return dynamic extent");

        CHECK(fixed.size() == 3U);
        CHECK(dyn.size() == 3U);
        CHECK(fixed.data() == data);
        CHECK(dyn.data() == data);
        check_span_content(fixed, expect);
        check_span_content(dyn, expect);
    }

    SUBCASE("last<N> and last(count)")
    {
        const auto fixed    = view.last<2>();
        const auto dyn      = view.last(2U);
        int        expect[] = {50, 60};

        using fixed_type = typename remove_cv_ref<decltype(fixed)>::type;
        using dyn_type   = typename remove_cv_ref<decltype(dyn)>::type;
        static_assert(std::is_same<fixed_type, ztl::span<int, 2>>::value,
                      "last<N>() must return fixed extent N");
        static_assert(std::is_same<dyn_type, ztl::span<int>>::value,
                      "last(count) must return dynamic extent");

        CHECK(fixed.size() == 2U);
        CHECK(dyn.size() == 2U);
        CHECK(fixed.data() == data + 4);
        CHECK(dyn.data() == data + 4);
        check_span_content(fixed, expect);
        check_span_content(dyn, expect);
    }

    SUBCASE("subspan<offset>() and subspan(offset)")
    {
        const auto fixed_tail = view.subspan<2>();
        const auto dyn_tail   = view.subspan(2U);
        int        expect[]   = {30, 40, 50, 60};

        using fixed_type = typename remove_cv_ref<decltype(fixed_tail)>::type;
        using dyn_type   = typename remove_cv_ref<decltype(dyn_tail)>::type;
        static_assert(std::is_same<fixed_type, ztl::span<int, 4>>::value,
                      "subspan<offset>() on fixed span must calculate fixed tail extent");
        static_assert(std::is_same<dyn_type, ztl::span<int>>::value,
                      "subspan(offset) must return dynamic extent");

        CHECK(fixed_tail.size() == 4U);
        CHECK(dyn_tail.size() == 4U);
        CHECK(fixed_tail.data() == data + 2);
        CHECK(dyn_tail.data() == data + 2);
        check_span_content(fixed_tail, expect);
        check_span_content(dyn_tail, expect);
    }

    SUBCASE("subspan<offset, count>() and subspan(offset, count)")
    {
        const auto fixed_mid = view.subspan<1, 3>();
        const auto dyn_mid   = view.subspan(1U, 3U);
        int        expect[]  = {20, 30, 40};

        using fixed_type = typename remove_cv_ref<decltype(fixed_mid)>::type;
        using dyn_type   = typename remove_cv_ref<decltype(dyn_mid)>::type;
        static_assert(std::is_same<fixed_type, ztl::span<int, 3>>::value,
                      "subspan<offset, count>() must return fixed extent count");
        static_assert(std::is_same<dyn_type, ztl::span<int>>::value,
                      "subspan(offset, count) must return dynamic extent");

        CHECK(fixed_mid.size() == 3U);
        CHECK(dyn_mid.size() == 3U);
        CHECK(fixed_mid.data() == data + 1);
        CHECK(dyn_mid.data() == data + 1);
        check_span_content(fixed_mid, expect);
        check_span_content(dyn_mid, expect);
    }

    SUBCASE("subviews alias the original storage")
    {
        auto mid = view.subspan<2, 2>();
        mid[0]   = 300;
        mid[1]   = 400;

        CHECK(data[2] == 300);
        CHECK(data[3] == 400);
    }

    SUBCASE("zero-length subviews from non-empty storage")
    {
        const auto first_zero = view.first<0>();
        const auto last_zero  = view.last<0>();
        const auto mid_zero   = view.subspan<3, 0>();
        const auto end_zero   = view.subspan<6>();

        CHECK(first_zero.size() == 0U);
        CHECK(last_zero.size() == 0U);
        CHECK(mid_zero.size() == 0U);
        CHECK(end_zero.size() == 0U);

        CHECK(first_zero.data() == data);
        CHECK(last_zero.data() == data + array_size(data));
        CHECK(mid_zero.data() == data + 3);
        CHECK(end_zero.data() == data + array_size(data));
    }
}

TEST_CASE("subviews from a dynamic-extent span")
{
    int            data[] = {1, 2, 3, 4, 5};
    ztl::span<int> view{data, array_size(data)};

    const auto first_two = view.first<2>();
    const auto last_two  = view.last<2>();
    const auto middle    = view.subspan<1, 3>();
    const auto tail      = view.subspan<2>();

    using first_type  = typename remove_cv_ref<decltype(first_two)>::type;
    using last_type   = typename remove_cv_ref<decltype(last_two)>::type;
    using middle_type = typename remove_cv_ref<decltype(middle)>::type;
    using tail_type   = typename remove_cv_ref<decltype(tail)>::type;

    static_assert(std::is_same<first_type, ztl::span<int, 2>>::value,
                  "first<N>() must return fixed extent even from dynamic span");
    static_assert(std::is_same<last_type, ztl::span<int, 2>>::value,
                  "last<N>() must return fixed extent even from dynamic span");
    static_assert(std::is_same<middle_type, ztl::span<int, 3>>::value,
                  "subspan<offset, count>() must return fixed extent from dynamic span");
    static_assert(std::is_same<tail_type, ztl::span<int>>::value,
                  "subspan<offset>() from dynamic span must remain dynamic extent");

    int first_expect[]  = {1, 2};
    int last_expect[]   = {4, 5};
    int middle_expect[] = {2, 3, 4};
    int tail_expect[]   = {3, 4, 5};

    check_span_content(first_two, first_expect);
    check_span_content(last_two, last_expect);
    check_span_content(middle, middle_expect);
    check_span_content(tail, tail_expect);
}

TEST_CASE("span preserves const and volatile element qualifiers")
{
    SUBCASE("const span exposes const elements")
    {
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

    SUBCASE("volatile span exposes volatile elements")
    {
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

TEST_CASE("as_bytes and as_writable_bytes expose byte-sized views")
{
    SUBCASE("as_bytes for fixed extent span has fixed byte extent")
    {
        std::uint16_t               data[] = {0x1122U, 0x3344U, 0x5566U};
        ztl::span<std::uint16_t, 3> words{data};

        auto bytes = ztl::as_bytes(words);

        using bytes_type = typename remove_cv_ref<decltype(bytes)>::type;
        static_assert(std::is_same<bytes_type, ztl::span<const ztl::byte, sizeof(data)>>::value,
                      "as_bytes(fixed span<T, N>) must return fixed byte extent sizeof(T) * N");

        CHECK(bytes.size() == sizeof(data));
        CHECK(bytes.size_bytes() == sizeof(data));
        CHECK(reinterpret_cast<const void*>(bytes.data()) == reinterpret_cast<const void*>(data));
    }

    SUBCASE("as_bytes for dynamic extent span has dynamic byte extent")
    {
        std::uint32_t            data[] = {1U, 2U};
        ztl::span<std::uint32_t> words{data, array_size(data)};

        auto bytes = ztl::as_bytes(words);

        using bytes_type = typename remove_cv_ref<decltype(bytes)>::type;
        static_assert(std::is_same<bytes_type, ztl::span<const ztl::byte>>::value,
                      "as_bytes(dynamic span) must return dynamic byte extent");

        CHECK(bytes.size() == sizeof(data));
        CHECK(bytes.size_bytes() == sizeof(data));
        CHECK(reinterpret_cast<const void*>(bytes.data()) == reinterpret_cast<const void*>(data));
    }

    SUBCASE("as_writable_bytes is available for mutable spans")
    {
        std::uint32_t               data[] = {0U, 1U};
        ztl::span<std::uint32_t, 2> words{data};

        auto bytes = ztl::as_writable_bytes(words);

        using bytes_type = typename remove_cv_ref<decltype(bytes)>::type;
        static_assert(std::is_same<bytes_type, ztl::span<ztl::byte, sizeof(data)>>::value,
                      "as_writable_bytes(fixed span<T, N>) must return mutable fixed byte span");

        CHECK(bytes.size() == sizeof(data));
        CHECK(bytes.size_bytes() == sizeof(data));
        CHECK(reinterpret_cast<void*>(bytes.data()) == reinterpret_cast<void*>(data));
    }

#if defined(ZTL_SPAN_TEST_BYTE_ALIASING_TARGET)
    // Target behaviour if you introduce ztl::object_byte for C++14 aliasing safety:
    // update the static_asserts above to use ztl::object_byte and enable this block.
    static_assert(std::is_same<ztl::object_byte, unsigned char>::value
                      || std::is_same<ztl::object_byte, std::byte>::value,
                  "object_byte should be a standard object-representation byte type");
#endif
}

TEST_CASE("span object representation is compact for embedded use")
{
    CHECK(sizeof(ztl::span<int, 4>) == sizeof(int*));
    CHECK(sizeof(ztl::span<int>) <= (sizeof(int*) + sizeof(std::size_t)));

    static_assert(std::is_copy_constructible<ztl::span<int>>::value,
                  "span must be copy constructible");
    static_assert(std::is_copy_assignable<ztl::span<int>>::value, "span must be copy assignable");
    static_assert(std::is_nothrow_copy_constructible<ztl::span<int>>::value,
                  "span copy construction should be noexcept");
    static_assert(std::is_nothrow_copy_assignable<ztl::span<int>>::value,
                  "span copy assignment should be noexcept");
}
