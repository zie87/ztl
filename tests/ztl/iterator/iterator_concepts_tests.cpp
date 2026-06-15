// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "ztl/iterator.hpp"

#include <doctest/doctest.h>

#include <forward_list>
#include <iterator>
#include <list>
#include <set>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

using ztl::is_bidirectional_iterator;
using ztl::is_contiguous_iterator;
using ztl::is_forward_iterator;
using ztl::is_input_iterator;
using ztl::is_iterator;
using ztl::is_output_iterator;
using ztl::is_random_access_iterator;
using ztl::is_sentinel_for;
using ztl::is_sized_sentinel_for;

// ===========================================================================
// Fixture iterators built on ztl::iterator_interface (mirrors of the
// iterator_interface_tests.cpp fixtures, so the traits are exercised against
// the very types the interface is meant to produce).
// ===========================================================================

struct IntFwd : ztl::iterator_interface<IntFwd, std::forward_iterator_tag, int> {
    IntFwd() : p_(nullptr) {}
    explicit IntFwd(int* p) : p_(p) {}
    int&    operator*() const { return *p_; }
    IntFwd& operator++() {
        ++p_;
        return *this;
    }
    bool operator==(IntFwd o) const { return p_ == o.p_; }

private:
    int* p_;
};

struct IntBidi : ztl::iterator_interface<IntBidi, std::bidirectional_iterator_tag, int> {
    IntBidi() : p_(nullptr) {}
    explicit IntBidi(int* p) : p_(p) {}
    int&     operator*() const { return *p_; }
    IntBidi& operator++() {
        ++p_;
        return *this;
    }
    IntBidi& operator--() {
        --p_;
        return *this;
    }
    bool operator==(IntBidi o) const { return p_ == o.p_; }

private:
    int* p_;
};

struct IntRA : ztl::iterator_interface<IntRA, std::random_access_iterator_tag, int> {
    IntRA() : p_(nullptr) {}
    explicit IntRA(int* p) : p_(p) {}
    int&   operator*() const { return *p_; }
    IntRA& operator+=(difference_type n) {
        p_ += n;
        return *this;
    }
    difference_type operator-(IntRA o) const { return p_ - o.p_; }
    bool            operator==(IntRA o) const { return p_ == o.p_; }

private:
    int* p_;
};

struct CountingInput : ztl::iterator_interface<CountingInput, std::input_iterator_tag, int, int> {
    CountingInput() : v_(0) {}
    explicit CountingInput(int v) : v_(v) {}
    int            operator*() const { return v_; }
    CountingInput& operator++() {
        ++v_;
        return *this;
    }
    bool operator==(CountingInput o) const { return v_ == o.v_; }

private:
    int v_;
};

struct FirstVal {
    int value;
};

// Proxy forward iterator: iterator_concept == forward_iterator_tag (capability),
// but std::iterator_traits category is downgraded to input because Reference is
// a by-value proxy.
struct PairFirstIter : ztl::proxy_iterator_interface<PairFirstIter, std::forward_iterator_tag, FirstVal> {
    PairFirstIter() : p_(nullptr) {}
    explicit PairFirstIter(std::pair<int, int>* p) : p_(p) {}
    FirstVal       operator*() const { return FirstVal{p_->first}; }
    PairFirstIter& operator++() {
        ++p_;
        return *this;
    }
    bool operator==(PairFirstIter o) const { return p_ == o.p_; }

private:
    std::pair<int, int>* p_;
};

// Proxy random-access iterator (the P2727 RepeatedChars example): capability is
// random-access, category is downgraded to input.
struct RepeatedChars
    : ztl::proxy_iterator_interface<RepeatedChars, std::random_access_iterator_tag, char, char> {
    RepeatedChars() : first_(nullptr), size_(0), n_(0) {}
    RepeatedChars(char const* first, difference_type size, difference_type n) : first_(first), size_(size), n_(n) {}
    char           operator*() const { return first_[n_ % size_]; }
    RepeatedChars& operator+=(difference_type i) {
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

// Contiguous iterator: advertises ztl::contiguous_iterator_tag as its concept.
struct IntContig : ztl::iterator_interface<IntContig, ztl::contiguous_iterator_tag, int> {
    IntContig() : p_(nullptr) {}
    explicit IntContig(int* p) : p_(p) {}
    int&       operator*() const { return *p_; }
    IntContig& operator+=(difference_type n) {
        p_ += n;
        return *this;
    }
    difference_type operator-(IntContig o) const { return p_ - o.p_; }
    bool            operator==(IntContig o) const { return p_ == o.p_; }

private:
    int* p_;
};

// Input iterator plus a heterogeneous, sized end-sentinel.  The iterator itself
// is *not* random-access, demonstrating that sized-sentinel support is
// independent of the iterator category.
struct Countdown : ztl::iterator_interface<Countdown, std::input_iterator_tag, int, int> {
    Countdown() : v_(0) {}
    explicit Countdown(int v) : v_(v) {}
    int        operator*() const { return v_; }
    Countdown& operator++() {
        ++v_;
        return *this;
    }
    bool operator==(Countdown o) const { return v_ == o.v_; }
    int  value() const { return v_; }

private:
    int v_;
};

struct EndSentinel {
    int stop;
};
inline bool operator==(Countdown i, EndSentinel s) { return i.value() == s.stop; }
inline bool operator==(EndSentinel s, Countdown i) { return i.value() == s.stop; }
inline bool operator!=(Countdown i, EndSentinel s) { return !(i == s); }
inline bool operator!=(EndSentinel s, Countdown i) { return !(i == s); }
inline int  operator-(EndSentinel s, Countdown i) { return s.stop - i.value(); }
inline int  operator-(Countdown i, EndSentinel s) { return i.value() - s.stop; }

struct NotAnIterator {
    int x;
};

// Degenerate types that are *almost* iterators — these previously provoked
// hard errors (rather than a clean false) in the trait machinery, so they are
// pinned here as regression cases.
struct OnlyDereference {
    int operator*() const;
};
struct OnlyIncrement {
    OnlyIncrement& operator++();
};

// ===========================================================================
// Tests
// ===========================================================================

TEST_SUITE("iterator traits — raw pointers") {

    TEST_CASE("mutable pointer is random-access and writable") {
        CHECK(is_random_access_iterator<int*>::value);
        CHECK(is_bidirectional_iterator<int*>::value);
        CHECK(is_forward_iterator<int*>::value);
        CHECK(is_input_iterator<int*>::value);
        CHECK(is_iterator<int*>::value);
        CHECK((is_output_iterator<int*, int>::value));
    }

    TEST_CASE("const pointer is readable but not writable") {
        CHECK(is_random_access_iterator<int const*>::value);
        CHECK(is_input_iterator<int const*>::value);
        CHECK_FALSE((is_output_iterator<int const*, int>::value));
    }

    TEST_CASE("void* is not an iterator") {
        CHECK_FALSE(is_iterator<void*>::value);
    }
}

TEST_SUITE("iterator traits — standard library iterators") {

    TEST_CASE("vector iterator is random-access") {
        CHECK(is_random_access_iterator<std::vector<int>::iterator>::value);
        CHECK(is_input_iterator<std::vector<int>::const_iterator>::value);
    }

    TEST_CASE("list iterator is bidirectional, not random-access") {
        CHECK(is_bidirectional_iterator<std::list<int>::iterator>::value);
        CHECK_FALSE(is_random_access_iterator<std::list<int>::iterator>::value);
    }

    TEST_CASE("forward_list iterator is forward, not bidirectional") {
        CHECK(is_forward_iterator<std::forward_list<int>::iterator>::value);
        CHECK_FALSE(is_bidirectional_iterator<std::forward_list<int>::iterator>::value);
    }

    TEST_CASE("set iterator is bidirectional") {
        CHECK(is_bidirectional_iterator<std::set<int>::iterator>::value);
    }

    TEST_CASE("istream_iterator is input only") {
        CHECK(is_input_iterator<std::istream_iterator<int>>::value);
        CHECK_FALSE(is_forward_iterator<std::istream_iterator<int>>::value);
    }

    TEST_CASE("output iterators are writable but not input") {
        CHECK(is_iterator<std::ostream_iterator<int>>::value);
        CHECK_FALSE(is_input_iterator<std::ostream_iterator<int>>::value);
        CHECK((is_output_iterator<std::ostream_iterator<int>, int>::value));
        CHECK((is_output_iterator<std::back_insert_iterator<std::vector<int>>, int>::value));
    }
}

TEST_SUITE("iterator traits — ztl::iterator_interface fixtures") {

    TEST_CASE("forward fixture") {
        CHECK(is_forward_iterator<IntFwd>::value);
        CHECK(is_input_iterator<IntFwd>::value);
        CHECK_FALSE(is_bidirectional_iterator<IntFwd>::value);
    }

    TEST_CASE("bidirectional fixture") {
        CHECK(is_bidirectional_iterator<IntBidi>::value);
        CHECK(is_forward_iterator<IntBidi>::value);
        CHECK_FALSE(is_random_access_iterator<IntBidi>::value);
    }

    TEST_CASE("random-access fixture satisfies the full operation set") {
        CHECK(is_random_access_iterator<IntRA>::value);
        CHECK(is_bidirectional_iterator<IntRA>::value);
    }

    TEST_CASE("input fixture is input only") {
        CHECK(is_input_iterator<CountingInput>::value);
        CHECK_FALSE(is_forward_iterator<CountingInput>::value);
    }
}

TEST_SUITE("iterator traits — proxy iterators use ITER_CONCEPT, not category") {

    // These document the deliberate split: the traits classify by declared
    // capability (iterator_concept), while std::iterator_traits reports the
    // conservative, downgraded category.

    TEST_CASE("proxy forward iterator is classified forward by capability") {
        CHECK(is_forward_iterator<PairFirstIter>::value);
        // ...even though its std category is downgraded to input:
        CHECK((std::is_same<std::iterator_traits<PairFirstIter>::iterator_category,
                            std::input_iterator_tag>::value));
    }

    TEST_CASE("proxy random-access iterator is classified random-access by capability") {
        CHECK(is_random_access_iterator<RepeatedChars>::value);
        CHECK((std::is_same<std::iterator_traits<RepeatedChars>::iterator_category,
                            std::input_iterator_tag>::value));
    }
}

TEST_SUITE("iterator traits — negatives and associated types") {

    TEST_CASE("non-iterator types are rejected at every tier") {
        CHECK_FALSE(is_iterator<int>::value);
        CHECK_FALSE(is_iterator<NotAnIterator>::value);
        CHECK_FALSE(is_input_iterator<double>::value);
        CHECK_FALSE(is_random_access_iterator<NotAnIterator>::value);
    }

    TEST_CASE("public associated-type aliases are pointer-safe") {
        CHECK((std::is_same<ztl::iter_value_t<int*>, int>::value));
        CHECK((std::is_same<ztl::iter_reference_t<int*>, int&>::value));
        CHECK((std::is_same<ztl::iter_difference_t<int*>, std::ptrdiff_t>::value));
        CHECK((std::is_same<ztl::iter_value_t<std::vector<int>::iterator>, int>::value));
        CHECK((std::is_same<ztl::iter_value_t<IntRA>, int>::value));
    }

    TEST_CASE("variable-template shorthands agree with the class traits") {
        CHECK(ztl::is_random_access_iterator_v<int*>);
        CHECK_FALSE(ztl::is_random_access_iterator_v<std::list<int>::iterator>);
        CHECK((ztl::is_output_iterator_v<int*, int>));
    }
}

TEST_SUITE("iterator traits — robustness on degenerate types") {

    // Each tier must instantiate to a clean `false` (never a hard error) for a
    // type that satisfies only part of the iterator requirements, including
    // arithmetic types where `a - b` is valid but yields no difference_type.

    TEST_CASE("type with operator* but no operator++") {
        CHECK_FALSE(is_iterator<OnlyDereference>::value);
        CHECK_FALSE(is_forward_iterator<OnlyDereference>::value);
        CHECK_FALSE(is_random_access_iterator<OnlyDereference>::value);
    }

    TEST_CASE("type with operator++ but no operator*") {
        CHECK_FALSE(is_iterator<OnlyIncrement>::value);
        CHECK_FALSE(is_random_access_iterator<OnlyIncrement>::value);
    }

    TEST_CASE("arithmetic type: a - b is valid but it is no iterator") {
        CHECK_FALSE(is_iterator<double>::value);
        CHECK_FALSE(is_random_access_iterator<double>::value);
        CHECK_FALSE(is_bidirectional_iterator<int>::value);
    }

    TEST_CASE("function pointer is not an iterator") {
        CHECK_FALSE(is_iterator<int (*)()>::value);
        CHECK_FALSE(is_random_access_iterator<int (*)()>::value);
    }
}

TEST_SUITE("iterator traits — contiguous iterators") {

    TEST_CASE("raw object pointers are contiguous") {
        CHECK(is_contiguous_iterator<int*>::value);
        CHECK(is_contiguous_iterator<int const*>::value);
        CHECK(is_random_access_iterator<int*>::value); // still satisfies the weaker tier
    }

    TEST_CASE("non-object pointers are not contiguous") {
        CHECK_FALSE(is_contiguous_iterator<void*>::value);
        CHECK_FALSE(is_contiguous_iterator<int (*)()>::value);
    }

    TEST_CASE("an iterator advertising contiguous_iterator_tag is contiguous") {
        CHECK(is_contiguous_iterator<IntContig>::value);
        CHECK(is_random_access_iterator<IntContig>::value);
    }

    TEST_CASE("random-access without the opt-in tag is not contiguous") {
        // IntRA is fully random-access but advertises only random_access_iterator_tag.
        CHECK(is_random_access_iterator<IntRA>::value);
        CHECK_FALSE(is_contiguous_iterator<IntRA>::value);
        // std library iterators do not advertise contiguity in C++14:
        CHECK_FALSE(is_contiguous_iterator<std::list<int>::iterator>::value);
    }

    TEST_CASE("proxy random-access iterator is not contiguous") {
        CHECK_FALSE(is_contiguous_iterator<RepeatedChars>::value);
    }

    TEST_CASE("non-iterators are not contiguous (no hard error)") {
        CHECK_FALSE(is_contiguous_iterator<int>::value);
        CHECK_FALSE(is_contiguous_iterator<NotAnIterator>::value);
    }
}

TEST_SUITE("iterator traits — sentinels") {

    TEST_CASE("an iterator is its own sentinel") {
        CHECK((is_sentinel_for<int*, int*>::value));
        CHECK((is_sentinel_for<IntFwd, IntFwd>::value));
        CHECK((is_sentinel_for<IntRA, IntRA>::value));
    }

    TEST_CASE("random-access iterators are sized sentinels for themselves") {
        CHECK((is_sized_sentinel_for<int*, int*>::value));
        CHECK((is_sized_sentinel_for<IntRA, IntRA>::value));
        CHECK((is_sized_sentinel_for<std::vector<int>::iterator, std::vector<int>::iterator>::value));
    }

    TEST_CASE("forward iterators are unsized sentinels for themselves") {
        CHECK((is_sentinel_for<IntFwd, IntFwd>::value));
        CHECK_FALSE((is_sized_sentinel_for<IntFwd, IntFwd>::value)); // no i - j
        CHECK_FALSE((is_sized_sentinel_for<std::list<int>::iterator, std::list<int>::iterator>::value));
    }

    TEST_CASE("heterogeneous sentinel: sized even though the iterator is input-only") {
        CHECK((is_sentinel_for<EndSentinel, Countdown>::value));
        CHECK((is_sized_sentinel_for<EndSentinel, Countdown>::value));
        CHECK_FALSE(is_random_access_iterator<Countdown>::value);
        // and it actually works at runtime:
        Countdown   it(3);
        EndSentinel end{8};
        CHECK((end - it) == 5);
        CHECK((it - end) == -5);
        CHECK(it != end);
    }

    TEST_CASE("a non-comparable type is not a sentinel") {
        CHECK_FALSE((is_sentinel_for<NotAnIterator, int*>::value));
        CHECK_FALSE((is_sentinel_for<int, int*>::value));
    }
}

// ---------------------------------------------------------------------------
// The traits are usable for SFINAE / tag dispatch: pick an overload purely by
// iterator capability, exactly as a generic algorithm would.
// ---------------------------------------------------------------------------
namespace {

template <typename It>
typename std::enable_if<is_random_access_iterator<It>::value, int>::type tier() {
    return 2;
}

template <typename It>
typename std::enable_if<is_bidirectional_iterator<It>::value && !is_random_access_iterator<It>::value, int>::type
tier() {
    return 1;
}

template <typename It>
typename std::enable_if<is_forward_iterator<It>::value && !is_bidirectional_iterator<It>::value, int>::type tier() {
    return 0;
}

} // namespace

TEST_SUITE("iterator traits — SFINAE overload selection") {

    TEST_CASE("overload resolves to the matching tier with no ambiguity") {
        CHECK(tier<int*>() == 2);
        CHECK(tier<std::list<int>::iterator>() == 1);
        CHECK(tier<std::forward_list<int>::iterator>() == 0);
        CHECK(tier<IntRA>() == 2);
        CHECK(tier<IntBidi>() == 1);
        CHECK(tier<IntFwd>() == 0);
    }
}
