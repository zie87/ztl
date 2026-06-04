/**
 * @file test_iterator_interface.cpp
 * @brief doctest test suite for iterator_interface (C++14 / IAR)
 *
 * Build (host GCC/Clang):
 *   g++ -std=c++14 -Wall -Wextra -pedantic -I. \
 *       test_iterator_interface.cpp -o test_iter && ./test_iter -v
 *
 * Build (IAR EWARM):
 *   iccarm --c++ --cpp_dialect c++14 --no_exceptions --no_rtti \
 *          -I. test_iterator_interface.cpp -o test_iter.o
 */

#include "ztl/iterator_interface.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <vector>

// ===========================================================================
// Fixture iterators
// ===========================================================================

// ---------------------------------------------------------------------------
// 1. Forward iterator over a raw int array
//    User provides: operator*(), operator++(), operator==()
// ---------------------------------------------------------------------------
struct IntFwd : ztl::iterator_interface<IntFwd, std::forward_iterator_tag, int>
{
    IntFwd() : p_(nullptr) {}
    explicit IntFwd(int* p) : p_(p) {}

    int&    operator*() const { return *p_; }
    IntFwd& operator++()
    {
        ++p_;
        return *this;
    }
    bool operator==(IntFwd o) const { return p_ == o.p_; }

private:
    int* p_;
};

// ---------------------------------------------------------------------------
// 2. Bidirectional iterator over a raw int array
//    User provides: operator*(), operator++(), operator--(), operator==()
// ---------------------------------------------------------------------------
struct IntBidi : ztl::iterator_interface<IntBidi, std::bidirectional_iterator_tag, int>
{
    IntBidi() : p_(nullptr) {}
    explicit IntBidi(int* p) : p_(p) {}

    int&     operator*() const { return *p_; }
    IntBidi& operator++()
    {
        ++p_;
        return *this;
    }
    IntBidi& operator--()
    {
        --p_;
        return *this;
    }
    bool operator==(IntBidi o) const { return p_ == o.p_; }

private:
    int* p_;
};

// ---------------------------------------------------------------------------
// 3. Random-access read-write iterator wrapping a raw int array
//    User provides: operator*(), operator+=(), operator-(), operator==()
//    Prefix ++/-- are synthesised from +=.
// ---------------------------------------------------------------------------
struct IntRA : ztl::iterator_interface<IntRA, std::random_access_iterator_tag, int>
{
    IntRA() : p_(nullptr) {}
    explicit IntRA(int* p) : p_(p) {}

    int&   operator*() const { return *p_; }
    IntRA& operator+=(difference_type n)
    {
        p_ += n;
        return *this;
    }
    difference_type operator-(IntRA o) const { return p_ - o.p_; }
    bool            operator==(IntRA o) const { return p_ == o.p_; }

private:
    int* p_;
};

// ---------------------------------------------------------------------------
// 4. RepeatedCharsIterator — the P2727R4 motivating example
//    Reference = char (by value); P2727 pointer is proxy_arrow_result<char>,
//    but operator-> is not generated because Reference is not a language reference.
// ---------------------------------------------------------------------------
struct RepeatedChars
: ztl::proxy_iterator_interface<RepeatedChars,
                                std::random_access_iterator_tag,
                                char, // ValueType
                                char> // Reference (proxy, by value)
{
    RepeatedChars() : first_(nullptr), size_(0), n_(0) {}
    RepeatedChars(char const* first, difference_type size, difference_type n)
    : first_(first), size_(size), n_(n)
    {
    }

    char           operator*() const { return first_[n_ % size_]; }
    RepeatedChars& operator+=(difference_type i)
    {
        n_ += i;
        return *this;
    }
    difference_type operator-(RepeatedChars o) const { return n_ - o.n_; }
    bool            operator==(RepeatedChars o) const { return first_ == o.first_ && n_ == o.n_; }

private:
    char const*     first_;
    difference_type size_;
    difference_type n_;
};

// ---------------------------------------------------------------------------
// 5. Proxy forward iterator — iterates pairs, yields only .first
// ---------------------------------------------------------------------------
struct FirstVal
{
    int value;
};

struct PairFirstIter
: ztl::proxy_iterator_interface<PairFirstIter,
                                std::forward_iterator_tag,
                                FirstVal> // Reference = FirstVal (by value)
{
    PairFirstIter() : p_(nullptr) {}
    explicit PairFirstIter(std::pair<int, int>* p) : p_(p) {}

    FirstVal       operator*() const { return FirstVal{p_->first}; }
    PairFirstIter& operator++()
    {
        ++p_;
        return *this;
    }
    bool operator==(PairFirstIter o) const { return p_ == o.p_; }

private:
    std::pair<int, int>* p_;
};

// ---------------------------------------------------------------------------
// 6. Input iterator (single-pass) — generates integers [start, end)
//    operator++(int) should return void per P2727.
// ---------------------------------------------------------------------------
struct CountingInput
: ztl::iterator_interface<CountingInput,
                          std::input_iterator_tag,
                          int,
                          int> // Reference = int (by value)
{
    CountingInput() : v_(0) {}
    explicit CountingInput(int v) : v_(v) {}

    int            operator*() const { return v_; }
    CountingInput& operator++()
    {
        ++v_;
        return *this;
    }
    bool operator==(CountingInput o) const { return v_ == o.v_; }

private:
    int v_;
};

// ---------------------------------------------------------------------------
// 7. Forward iterator over a struct — tests synthesised operator->
// ---------------------------------------------------------------------------
struct Point
{
    float x, y;
};

struct PointIter : ztl::iterator_interface<PointIter, std::forward_iterator_tag, Point>
{
    PointIter() : p_(nullptr) {}
    explicit PointIter(Point* p) : p_(p) {}

    Point&     operator*() const { return *p_; }
    PointIter& operator++()
    {
        ++p_;
        return *this;
    }
    bool operator==(PointIter o) const { return p_ == o.p_; }

private:
    Point* p_;
};

// ---------------------------------------------------------------------------
// 8. Const random-access iterator (read-only, Reference = const int&)
// ---------------------------------------------------------------------------
struct ConstIntRA
: ztl::iterator_interface<ConstIntRA, std::random_access_iterator_tag, int, int const&, int const*>
{
    ConstIntRA() : p_(nullptr) {}
    explicit ConstIntRA(int const* p) : p_(p) {}

    int const&  operator*() const { return *p_; }
    ConstIntRA& operator+=(difference_type n)
    {
        p_ += n;
        return *this;
    }
    difference_type operator-(ConstIntRA o) const { return p_ - o.p_; }
    bool            operator==(ConstIntRA o) const { return p_ == o.p_; }

private:
    int const* p_;
};

// ===========================================================================
// Tests
// ===========================================================================

TEST_SUITE("nested typedefs")
{

    TEST_CASE("forward iterator typedefs")
    {
        CHECK((std::is_same<IntFwd::value_type, int>::value));
        CHECK((std::is_same<IntFwd::reference, int&>::value));
        CHECK((std::is_same<IntFwd::pointer, int*>::value));
        CHECK((std::is_same<IntFwd::difference_type, std::ptrdiff_t>::value));
        CHECK((std::is_same<IntFwd::iterator_category, std::forward_iterator_tag>::value));
    }

    TEST_CASE("random-access iterator typedefs")
    {
        CHECK((std::is_same<IntRA::iterator_category, std::random_access_iterator_tag>::value));
    }

    TEST_CASE("proxy iterator has P2727 proxy pointer and input category")
    {
        CHECK((std::is_same<PairFirstIter::pointer, ztl::proxy_arrow_result<FirstVal>>::value));
        CHECK((std::is_same<PairFirstIter::iterator_category, std::input_iterator_tag>::value));
    }

    TEST_CASE("iterator_traits picks up typedefs")
    {
        using Traits = std::iterator_traits<IntRA>;
        CHECK((std::is_same<Traits::value_type, int>::value));
        CHECK((std::is_same<Traits::iterator_category, std::random_access_iterator_tag>::value));
    }
    TEST_CASE("value_type strips cv qualification")
    {
        struct ConstValueIter
        : ztl::iterator_interface<ConstValueIter,
                                  std::forward_iterator_tag,
                                  int const,
                                  int const&,
                                  int const*>
        {
            ConstValueIter() : p_(nullptr) {}
            explicit ConstValueIter(int const* p) : p_(p) {}
            int const&      operator*() const { return *p_; }
            ConstValueIter& operator++()
            {
                ++p_;
                return *this;
            }
            bool operator==(ConstValueIter o) const { return p_ == o.p_; }

        private:
            int const* p_;
        };

        CHECK((std::is_same<ConstValueIter::value_type, int>::value));
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("forward iterator")
{

    TEST_CASE("traversal and sum")
    {
        int    arr[] = {10, 20, 30, 40, 50};
        IntFwd first(arr), last(arr + 5);
        int    sum = 0;
        for (auto it = first; it != last; ++it)
            sum += *it;
        CHECK(sum == 150);
    }

    TEST_CASE("post-increment returns copy")
    {
        int    arr[] = {1, 2, 3};
        IntFwd it(arr);
        IntFwd old = it++;
        CHECK(*old == 1);
        CHECK(*it == 2);
    }

    TEST_CASE("operator== and operator!=")
    {
        int    arr[] = {1, 2};
        IntFwd a(arr), b(arr), c(arr + 1);
        CHECK(a == b);
        CHECK(!(a != b));
        CHECK(a != c);
    }

    TEST_CASE("operator-> on struct type")
    {
        Point pts[] = {
            {1.f, 2.f},
            {3.f, 4.f}
        };
        PointIter it(pts);
        CHECK(it->x == doctest::Approx(1.f));
        CHECK(it->y == doctest::Approx(2.f));
        ++it;
        CHECK(it->x == doctest::Approx(3.f));
    }

    TEST_CASE("std::copy")
    {
        int src[]  = {5, 6, 7, 8};
        int dst[4] = {};
        std::copy(IntFwd(src), IntFwd(src + 4), dst);
        for (int i = 0; i < 4; ++i)
            CHECK(dst[i] == src[i]);
    }

    TEST_CASE("std::find")
    {
        int  arr[] = {3, 1, 4, 1, 5, 9};
        auto it    = std::find(IntFwd(arr), IntFwd(arr + 6), 5);
        CHECK(*it == 5);
    }

    TEST_CASE("std::fill")
    {
        int arr[5] = {};
        std::fill(IntFwd(arr), IntFwd(arr + 5), 7);
        for (int i = 0; i < 5; ++i)
            CHECK(arr[i] == 7);
    }

    TEST_CASE("std::distance (linear)")
    {
        int arr[7];
        CHECK(std::distance(IntFwd(arr), IntFwd(arr + 7)) == 7);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("bidirectional iterator")
{

    TEST_CASE("forward then backward")
    {
        int     arr[] = {1, 2, 3, 4, 5};
        IntBidi first(arr), last(arr + 5);

        std::vector<int> fwd;
        for (auto it = first; it != last; ++it)
            fwd.push_back(*it);

        std::vector<int> bwd;
        auto             it = last;
        while (it != first)
        {
            --it;
            bwd.push_back(*it);
        }

        REQUIRE(fwd.size() == 5u);
        for (int i = 0; i < 5; ++i)
            CHECK(fwd[i] == bwd[4 - i]);
    }

    TEST_CASE("post-decrement returns copy")
    {
        int     arr[] = {10, 20, 30};
        IntBidi it(arr + 2);
        IntBidi old = it--;
        CHECK(*old == 30);
        CHECK(*it == 20);
    }

    TEST_CASE("std::reverse")
    {
        int arr[] = {1, 2, 3, 4, 5};
        std::reverse(IntBidi(arr), IntBidi(arr + 5));
        CHECK(arr[0] == 5);
        CHECK(arr[4] == 1);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("random-access iterator — IntRA")
{

    TEST_CASE("prefix ++ and -- synthesised from +=")
    {
        int   arr[] = {10, 20, 30};
        IntRA it(arr);
        ++it;
        CHECK(*it == 20);
        ++it;
        CHECK(*it == 30);
        --it;
        CHECK(*it == 20);
    }

    TEST_CASE("post-increment / post-decrement")
    {
        int   arr[] = {1, 2, 3};
        IntRA it(arr);
        IntRA old = it++;
        CHECK(*old == 1);
        CHECK(*it == 2);
        old = it--;
        CHECK(*old == 2);
        CHECK(*it == 1);
    }

    TEST_CASE("operator+=, -=")
    {
        int   arr[] = {0, 1, 2, 3, 4};
        IntRA it(arr);
        it += 3;
        CHECK(*it == 3);
        it -= 2;
        CHECK(*it == 1);
    }

    TEST_CASE("operator+ commutative, operator- scalar")
    {
        int   arr[] = {10, 20, 30};
        IntRA it(arr);
        CHECK(*(it + 2) == 30);
        CHECK(*(2 + it) == 30);
        CHECK(*(it + 2 - 1) == 20);
    }

    TEST_CASE("operator[]")
    {
        int   arr[] = {0, 1, 2, 3, 4};
        IntRA it(arr);
        for (int i = 0; i < 5; ++i)
            CHECK(it[i] == i);
    }

    TEST_CASE("write through operator[]")
    {
        int   arr[4] = {};
        IntRA it(arr);
        for (int i = 0; i < 4; ++i)
            it[i] = i * i;
        CHECK(arr[0] == 0);
        CHECK(arr[1] == 1);
        CHECK(arr[2] == 4);
        CHECK(arr[3] == 9);
    }

    TEST_CASE("operator-(iter,iter)")
    {
        int   arr[10];
        IntRA first(arr), last(arr + 10);
        CHECK((last - first) == 10);
        CHECK((first - last) == -10);
    }

    TEST_CASE("comparison operators")
    {
        int   arr[3];
        IntRA a(arr), b(arr + 1), c(arr);
        CHECK(a == c);
        CHECK(a != b);
        CHECK(a < b);
        CHECK(b > a);
        CHECK(a <= c);
        CHECK(a <= b);
        CHECK(b >= a);
        CHECK(a >= c);
    }

    TEST_CASE("std::sort")
    {
        int arr[] = {5, 3, 1, 4, 2};
        std::sort(IntRA(arr), IntRA(arr + 5));
        for (int i = 0; i < 5; ++i)
            CHECK(arr[i] == i + 1);
    }

    TEST_CASE("std::lower_bound")
    {
        int  arr[] = {1, 3, 5, 7, 9};
        auto it    = std::lower_bound(IntRA(arr), IntRA(arr + 5), 5);
        CHECK(*it == 5);
        CHECK((it - IntRA(arr)) == 2);
    }

    TEST_CASE("std::distance O(1)")
    {
        int arr[8];
        CHECK(std::distance(IntRA(arr), IntRA(arr + 8)) == 8);
    }

    TEST_CASE("std::advance forward and backward")
    {
        int   arr[] = {10, 20, 30, 40};
        IntRA it(arr);
        std::advance(it, 3);
        CHECK(*it == 40);
        std::advance(it, -2);
        CHECK(*it == 20);
    }

    TEST_CASE("std::next / std::prev")
    {
        int   arr[] = {10, 20, 30, 40};
        IntRA it(arr);
        CHECK(*std::next(it, 2) == 30);
        CHECK(*std::prev(IntRA(arr + 3)) == 30);
    }

    TEST_CASE("std::equal")
    {
        int a[] = {1, 2, 3}, b[] = {1, 2, 3};
        CHECK(std::equal(IntRA(a), IntRA(a + 3), IntRA(b)));
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("RepeatedChars — P2727R4 motivating example")
{

    TEST_CASE("std::copy reproduces paper result")
    {
        char const    pat[] = "foo";
        RepeatedChars first(pat, 3, 0), last(pat, 3, 7);
        char          buf[8] = {};
        std::copy(first, last, buf);
        buf[7] = '\0';
        CHECK(std::string(buf) == "foofoof");
    }

    TEST_CASE("operator[]")
    {
        char const    p[] = "abc";
        RepeatedChars it(p, 3, 0);
        CHECK(it[0] == 'a');
        CHECK(it[1] == 'b');
        CHECK(it[2] == 'c');
        CHECK(it[3] == 'a'); // wraps
        CHECK(it[5] == 'c');
    }

    TEST_CASE("arithmetic")
    {
        char const    p[] = "xy";
        RepeatedChars it(p, 2, 0);
        CHECK(*(it + 1) == 'y');
        CHECK(*(it + 3) == 'y'); // 3%2==1
        CHECK(*(2 + it) == 'x'); // 2%2==0
    }

    TEST_CASE("difference")
    {
        char const    p[] = "ab";
        RepeatedChars a(p, 2, 0), b(p, 2, 5);
        CHECK((b - a) == 5);
        CHECK((a - b) == -5);
    }

    TEST_CASE("comparisons")
    {
        char const    p[] = "z";
        RepeatedChars a(p, 1, 0), b(p, 1, 1), c(p, 1, 0);
        CHECK(a == c);
        CHECK(a != b);
        CHECK(a < b);
        CHECK(b > a);
        CHECK(a <= c);
        CHECK(b >= a);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("proxy_iterator_interface")
{

    TEST_CASE("dereferences to proxy value")
    {
        std::pair<int, int> pairs[] = {
            {10, 1},
            {20, 2},
            {30, 3}
        };
        PairFirstIter first(pairs), last(pairs + 3);

        std::vector<int> vals;
        for (auto it = first; it != last; ++it)
            vals.push_back((*it).value);

        REQUIRE(vals.size() == 3u);
        CHECK(vals[0] == 10);
        CHECK(vals[1] == 20);
        CHECK(vals[2] == 30);
    }

    TEST_CASE("post-increment")
    {
        std::pair<int, int> pairs[] = {
            {1, 0},
            {2, 0}
        };
        PairFirstIter it(pairs);
        auto          old = it++;
        CHECK((*old).value == 1);
        CHECK((*it).value == 2);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("input iterator")
{

    TEST_CASE("operator++(int) returns void")
    {
        CountingInput ci(0);
        using PostIncT = decltype(ci++);
        CHECK((std::is_same<PostIncT, void>::value));
    }

    TEST_CASE("single-pass summation")
    {
        CountingInput first(0), last(5);
        int           sum = 0;
        for (auto it = first; it != last; ++it)
            sum += *it;
        CHECK(sum == 10); // 0+1+2+3+4
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("const random-access iterator")
{

    TEST_CASE("read-only traversal")
    {
        int const  arr[] = {10, 20, 30};
        ConstIntRA it(arr);
        CHECK(*it == 10);
        ++it;
        CHECK(*it == 20);
        CHECK(it[1] == 30);
    }

    TEST_CASE("std::lower_bound on const data")
    {
        int const arr[] = {2, 4, 6, 8};
        auto      it    = std::lower_bound(ConstIntRA(arr), ConstIntRA(arr + 4), 6);
        CHECK(*it == 6);
    }

    TEST_CASE("operator-> on const iterator")
    {
        // ConstIntRA has Pointer = int const*, Reference = int const&
        // so operator-> should return int const*
        int const  arr[] = {42};
        ConstIntRA it(arr);
        int const* p = it.operator->();
        CHECK(*p == 42);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("proxy_arrow_result")
{

    TEST_CASE("wraps value and provides ->")
    {
        struct Vec2
        {
            float x, y;
        };
        ztl::proxy_arrow_result<Vec2> par(Vec2{1.f, 2.f});
        CHECK(par->x == doctest::Approx(1.f));
        CHECK(par->y == doctest::Approx(2.f));
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("base_reference adaptation")
{

    struct AdaptRA : ztl::iterator_interface<AdaptRA, std::random_access_iterator_tag, int>
    {
        AdaptRA() : p_(nullptr) {}
        explicit AdaptRA(int* p) : p_(p) {}

    private:
        friend struct ztl::iterator_interface_access;
        int*&       base_reference() { return p_; }
        int* const& base_reference() const { return p_; }
        int*        p_;
    };

    TEST_CASE("synthesises operations from base_reference")
    {
        int     arr[] = {1, 2, 3, 4};
        AdaptRA first(arr), last(arr + 4);
        CHECK(*first == 1);
        CHECK(*(first + 2) == 3);
        CHECK((last - first) == 4);
        CHECK(first == AdaptRA(arr));
        CHECK(first < last);
    }
}

// ---------------------------------------------------------------------------
TEST_SUITE("edge cases")
{

    TEST_CASE("default-constructed iterators compare equal (forward)")
    {
        IntFwd a, b;
        CHECK(a == b);
    }

    TEST_CASE("default-constructed iterators compare equal (RA)")
    {
        IntRA a, b;
        CHECK(a == b);
    }

    TEST_CASE("single-element range")
    {
        int   arr[] = {42};
        IntRA first(arr), last(arr + 1);
        CHECK(*first == 42);
        CHECK(std::distance(first, last) == 1);
        CHECK(first != last);
        ++first;
        CHECK(first == last);
    }

    TEST_CASE("empty range")
    {
        int   arr[1];
        IntRA it(arr);
        CHECK(std::distance(it, it) == 0);
    }

    TEST_CASE("large std::sort")
    {
        std::vector<int> v(200);
        for (int i = 0; i < 200; ++i)
            v[i] = 200 - i;
        std::sort(IntRA(v.data()), IntRA(v.data() + 200));
        for (int i = 0; i < 200; ++i)
            CHECK(v[i] == i + 1);
    }
}
