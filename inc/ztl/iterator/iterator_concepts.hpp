// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_ITERATOR_ITERATORCONCEPTS_HPP
#define ZTL_ITERATOR_ITERATORCONCEPTS_HPP

#include "ztl/iterator/iterator_tags.hpp"
#include "ztl/type_traits.hpp"
#include "ztl/memory.hpp"

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

// ===========================================================================
// C++14 trait emulation of the C++20 iterator concepts.
//
// These are *traits*, not concepts: each is a class derived from
// ztl::bool_constant and is usable in SFINAE, static_assert and tag dispatch.
// They are modelled on std::input_iterator ... std::random_access_iterator,
// but expressed with the C++14 toolbox already present in this library
// (void_t SFINAE + std::declval).
//
// Faithfulness vs. simplification (read this before trusting an edge case):
//   * Iterator tier is decided by ITER_CONCEPT(I): the member typedef
//     `iterator_concept` if present, otherwise `iterator_category`, otherwise
//     std::random_access_iterator_tag for pointers. This mirrors the rule the
//     standard library uses for the C++20 concepts, so a proxy iterator that
//     advertises `iterator_concept = random_access_iterator_tag` is classified
//     by capability here even though its std::iterator_traits category is
//     intentionally downgraded to input (see iterator_interface.hpp). The two
//     views are meant to disagree: std algorithms read the conservative
//     category; these traits read the declared capability.
//   * The std concepts also pull in common_reference / indirectly_readable /
//     weakly_incrementable machinery that is impractical to reproduce in C++14.
//     We approximate "readable" as "*i is well-formed and not void" and
//     "incrementable" as "++i yields I& and i++ is well-formed". This is strong
//     enough for tag dispatch and for guarding the algorithms in this library,
//     but it is deliberately not a bit-for-bit reimplementation of [iterator.concepts].
// ===========================================================================

namespace ztl {

namespace details {

// ---------------------------------------------------------------------------
// Associated-type access that is safe to instantiate for *any* T.
//
// std::iterator_traits<T> may hard-error (not SFINAE) for a non-iterator T on
// some C++14 standard libraries, so we read member typedefs directly and
// special-case pointers instead of going through std::iterator_traits.
// ---------------------------------------------------------------------------
template <typename I, typename = void>
struct iter_difference {};
template <typename I>
struct iter_difference<I, void_t<typename I::difference_type>> {
    typedef typename I::difference_type type;
};
template <typename T>
struct iter_difference<T*, void> {
    typedef std::ptrdiff_t type;
};

template <typename I, typename = void>
struct iter_value {};
template <typename I>
struct iter_value<I, void_t<typename I::value_type>> {
    typedef typename I::value_type type;
};
template <typename T>
struct iter_value<T*, void> {
    typedef remove_cv_t<T> type;
};

// ITER_CONCEPT(I): iterator_concept, else iterator_category, else (pointer) RA.
// The category/pointer fallback lives in iter_concept_cat; iter_concept_impl
// then prefers a member iterator_concept via a more-specialized partial
// specialization (so a type having *both* typedefs is unambiguous).
template <typename I, typename = void>
struct iter_concept_cat {};
template <typename I>
struct iter_concept_cat<I, void_t<typename I::iterator_category>> {
    typedef typename I::iterator_category type;
};
template <typename T>
struct iter_concept_cat<T*, void> {
    typedef ::ztl::contiguous_iterator_tag type; // pointers are contiguous (cf. C++20 iterator_traits<T*>)
};

template <typename I, typename = void>
struct iter_concept_impl : iter_concept_cat<I> {};
template <typename I>
struct iter_concept_impl<I, void_t<typename I::iterator_concept>> { // strictly preferred
    typedef typename I::iterator_concept type;
};

template <typename I, typename = void>
struct has_iter_concept : std::false_type {};
template <typename I>
struct has_iter_concept<I, void_t<typename iter_concept_impl<I>::type>> : std::true_type {};

// Derived-from-tag test that is false (instead of ill-formed) when I has no tag.
template <typename I, typename Tag, bool = has_iter_concept<I>::value>
struct concept_at_least : std::false_type {};
template <typename I, typename Tag>
struct concept_at_least<I, Tag, true>
    : bool_constant<std::is_base_of<Tag, typename iter_concept_impl<I>::type>::value> {};

} // namespace details

// Public associated-type aliases (handy and pointer-safe).
template <typename I>
using iter_difference_t = typename details::iter_difference<I>::type;

template <typename I>
using iter_value_t = typename details::iter_value<I>::type;

template <typename I>
using iter_reference_t = decltype(*std::declval<I&>());

namespace details {

// ---------------------------------------------------------------------------
// Expression detectors. Each is always instantiable -> resolves to a bool trait.
// ---------------------------------------------------------------------------
template <typename I, typename = void>
struct is_dereferenceable : std::false_type {};
template <typename I>
struct is_dereferenceable<I, void_t<decltype(*std::declval<I&>())>>
    : bool_constant<!std::is_void<decltype(*std::declval<I&>())>::value> {};

template <typename I, typename = void>
struct has_preincrement : std::false_type {};
template <typename I>
struct has_preincrement<I, void_t<decltype(++std::declval<I&>())>>
    : bool_constant<std::is_same<decltype(++std::declval<I&>()), I&>::value> {};

template <typename I, typename = void>
struct has_postincrement : std::false_type {};
template <typename I>
struct has_postincrement<I, void_t<decltype(std::declval<I&>()++)>> : std::true_type {};

// Forward's multipass requirement: i++ must be convertible to I.  Kept as a
// dedicated detector so it can be used inside a bool_constant for a type that
// has no operator++ at all (a raw decltype there would be a hard error).
template <typename I, typename = void>
struct postincrement_is_iter : std::false_type {};
template <typename I>
struct postincrement_is_iter<I, void_t<decltype(std::declval<I&>()++)>>
    : bool_constant<std::is_convertible<decltype(std::declval<I&>()++), I>::value> {};

template <typename I, typename = void>
struct has_predecrement : std::false_type {};
template <typename I>
struct has_predecrement<I, void_t<decltype(--std::declval<I&>())>>
    : bool_constant<std::is_same<decltype(--std::declval<I&>()), I&>::value> {};

template <typename I, typename = void>
struct has_postdecrement : std::false_type {};
template <typename I>
struct has_postdecrement<I, void_t<decltype(std::declval<I&>()--)>>
    : bool_constant<std::is_convertible<decltype(std::declval<I&>()--), I>::value> {};

template <typename I, typename = void>
struct has_equality : std::false_type {};
template <typename I>
struct has_equality<I, void_t<decltype(std::declval<I const&>() == std::declval<I const&>()),
                              decltype(std::declval<I const&>() != std::declval<I const&>())>>
    : bool_constant<std::is_convertible<decltype(std::declval<I const&>() == std::declval<I const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() != std::declval<I const&>()), bool>::value> {
};

template <typename I, typename = void>
struct has_difference : std::false_type {};
template <typename I>
struct has_difference<
    I, void_t<typename iter_difference<I>::type, decltype(std::declval<I const&>() - std::declval<I const&>())>>
    : bool_constant<std::is_convertible<decltype(std::declval<I const&>() - std::declval<I const&>()),
                                        iter_difference_t<I>>::value> {};

template <typename I, typename = void>
struct has_plus_equal : std::false_type {};
template <typename I>
struct has_plus_equal<I, void_t<decltype(std::declval<I&>() += std::declval<iter_difference_t<I>>())>>
    : bool_constant<std::is_same<decltype(std::declval<I&>() += std::declval<iter_difference_t<I>>()), I&>::value> {};

template <typename I, typename = void>
struct has_minus_equal : std::false_type {};
template <typename I>
struct has_minus_equal<I, void_t<decltype(std::declval<I&>() -= std::declval<iter_difference_t<I>>())>>
    : bool_constant<std::is_same<decltype(std::declval<I&>() -= std::declval<iter_difference_t<I>>()), I&>::value> {};

template <typename I, typename = void>
struct has_plus : std::false_type {};
template <typename I>
struct has_plus<I, void_t<decltype(std::declval<I const&>() + std::declval<iter_difference_t<I>>()),
                          decltype(std::declval<iter_difference_t<I>>() + std::declval<I const&>())>>
    : bool_constant<
          std::is_convertible<decltype(std::declval<I const&>() + std::declval<iter_difference_t<I>>()), I>::value &&
          std::is_convertible<decltype(std::declval<iter_difference_t<I>>() + std::declval<I const&>()), I>::value> {};

template <typename I, typename = void>
struct has_minus_n : std::false_type {};
template <typename I>
struct has_minus_n<I, void_t<decltype(std::declval<I const&>() - std::declval<iter_difference_t<I>>())>>
    : bool_constant<
          std::is_convertible<decltype(std::declval<I const&>() - std::declval<iter_difference_t<I>>()), I>::value> {};

template <typename I, typename = void>
struct has_subscript : std::false_type {};
template <typename I>
struct has_subscript<I, void_t<decltype(std::declval<I const&>()[std::declval<iter_difference_t<I>>()])>>
    : std::true_type {};

template <typename I, typename = void>
struct has_ordering : std::false_type {};
template <typename I>
struct has_ordering<I, void_t<decltype(std::declval<I const&>() < std::declval<I const&>()),
                              decltype(std::declval<I const&>() > std::declval<I const&>()),
                              decltype(std::declval<I const&>() <= std::declval<I const&>()),
                              decltype(std::declval<I const&>() >= std::declval<I const&>())>>
    : bool_constant<std::is_convertible<decltype(std::declval<I const&>() < std::declval<I const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() > std::declval<I const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() <= std::declval<I const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() >= std::declval<I const&>()), bool>::value> {
};

// Writable<I, T>: *i = t and *i++ = t are valid (output requirement).
template <typename I, typename T, typename = void>
struct is_writable : std::false_type {};
template <typename I, typename T>
struct is_writable<
    I, T,
    void_t<decltype(*std::declval<I&>() = std::declval<T>()), decltype(*std::declval<I&>()++ = std::declval<T>())>>
    : std::true_type {};

// ---------------------------------------------------------------------------
// Tier definitions (each builds on the previous one).
// ---------------------------------------------------------------------------
template <typename I>
struct input_or_output_iterator
    : bool_constant<std::is_copy_constructible<I>::value && std::is_copy_assignable<I>::value &&
                    is_dereferenceable<I>::value && has_preincrement<I>::value && has_postincrement<I>::value &&
                    has_iter_concept<I>::value> {};

template <typename I>
struct input_iterator : bool_constant<input_or_output_iterator<I>::value && has_equality<I>::value &&
                                      concept_at_least<I, std::input_iterator_tag>::value> {};

template <typename I>
struct forward_iterator
    : bool_constant<input_iterator<I>::value && concept_at_least<I, std::forward_iterator_tag>::value &&
                    std::is_default_constructible<I>::value && postincrement_is_iter<I>::value> {};

template <typename I>
struct bidirectional_iterator
    : bool_constant<forward_iterator<I>::value && concept_at_least<I, std::bidirectional_iterator_tag>::value &&
                    has_predecrement<I>::value && has_postdecrement<I>::value> {};

template <typename I>
struct random_access_iterator
    : bool_constant<bidirectional_iterator<I>::value && concept_at_least<I, std::random_access_iterator_tag>::value &&
                    has_plus_equal<I>::value && has_minus_equal<I>::value && has_plus<I>::value &&
                    has_minus_n<I>::value && has_difference<I>::value && has_subscript<I>::value &&
                    has_ordering<I>::value> {};

// ---------------------------------------------------------------------------
// Contiguous iterator support.
//
// Contiguity cannot be deduced from operations (a std::deque iterator is
// random-access with lvalue references and a working to_address yet is not
// contiguous), so it requires the explicit ztl::contiguous_iterator_tag
// opt-in, exactly as C++20 requires std::contiguous_iterator_tag.
// ---------------------------------------------------------------------------
template <typename I, typename = void>
struct has_iter_value : std::false_type {};
template <typename I>
struct has_iter_value<I, void_t<typename iter_value<I>::type>> : std::true_type {};

// ztl::to_address(i) is well-formed and yields a pointer to the element.
template <typename I, typename = void>
struct to_addressable : std::false_type {};
template <typename I>
struct to_addressable<I, void_t<decltype(::ztl::to_address(std::declval<I const&>()))>>
    : bool_constant<std::is_same<decltype(::ztl::to_address(std::declval<I const&>())),
                                 typename std::add_pointer<iter_reference_t<I>>::type>::value> {};

// The structural part is only instantiated once I is known to be random-access
// *and* to have a value type, so iter_value_t / iter_reference_t are safe here.
template <typename I, bool = random_access_iterator<I>::value && has_iter_value<I>::value>
struct contiguous_iterator : std::false_type {};
template <typename I>
struct contiguous_iterator<I, true>
    : bool_constant<concept_at_least<I, ::ztl::contiguous_iterator_tag>::value &&
                    std::is_lvalue_reference<iter_reference_t<I>>::value &&
                    std::is_same<iter_value_t<I>, remove_cvref_t<iter_reference_t<I>>>::value &&
                    to_addressable<I>::value> {};

// ---------------------------------------------------------------------------
// Sentinel support.  A sentinel S for I is a semiregular type that is
// (heterogeneously) equality-comparable with I.  A sized sentinel additionally
// supports s - i and i - s, both yielding iter_difference_t<I>.
// ---------------------------------------------------------------------------
template <typename S, typename I, typename = void>
struct weakly_equality_comparable : std::false_type {};
template <typename S, typename I>
struct weakly_equality_comparable<S, I,
                                  void_t<decltype(std::declval<I const&>() == std::declval<S const&>()),
                                         decltype(std::declval<S const&>() == std::declval<I const&>()),
                                         decltype(std::declval<I const&>() != std::declval<S const&>()),
                                         decltype(std::declval<S const&>() != std::declval<I const&>())>>
    : bool_constant<std::is_convertible<decltype(std::declval<I const&>() == std::declval<S const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<S const&>() == std::declval<I const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() != std::declval<S const&>()), bool>::value &&
                    std::is_convertible<decltype(std::declval<S const&>() != std::declval<I const&>()), bool>::value> {
};

template <typename S, typename I, typename = void>
struct has_sized_sentinel_difference : std::false_type {};
template <typename S, typename I>
struct has_sized_sentinel_difference<
    S, I,
    void_t<typename iter_difference<I>::type, decltype(std::declval<S const&>() - std::declval<I const&>()),
           decltype(std::declval<I const&>() - std::declval<S const&>())>>
    : bool_constant<std::is_convertible<decltype(std::declval<S const&>() - std::declval<I const&>()),
                                        iter_difference_t<I>>::value &&
                    std::is_convertible<decltype(std::declval<I const&>() - std::declval<S const&>()),
                                        iter_difference_t<I>>::value> {};

template <typename S, typename I>
struct sentinel_for : bool_constant<input_or_output_iterator<I>::value && std::is_copy_constructible<S>::value &&
                                    std::is_copy_assignable<S>::value && std::is_default_constructible<S>::value &&
                                    weakly_equality_comparable<S, I>::value> {};

template <typename S, typename I>
struct sized_sentinel_for : bool_constant<sentinel_for<S, I>::value && has_sized_sentinel_difference<S, I>::value> {};

} // namespace details

// ===========================================================================
// Public traits.
// ===========================================================================

// std::input_or_output_iterator equivalent.
template <typename I>
struct is_iterator : details::input_or_output_iterator<I> {};

template <typename I>
struct is_input_iterator : details::input_iterator<I> {};

// std::output_iterator<I, T>: writes a T through *i and *i++.
template <typename I, typename T>
struct is_output_iterator
    : bool_constant<details::input_or_output_iterator<I>::value && details::is_writable<I, T>::value> {};

template <typename I>
struct is_forward_iterator : details::forward_iterator<I> {};

template <typename I>
struct is_bidirectional_iterator : details::bidirectional_iterator<I> {};

template <typename I>
struct is_random_access_iterator : details::random_access_iterator<I> {};

// std::contiguous_iterator equivalent (requires the ztl::contiguous_iterator_tag
// opt-in; raw object pointers qualify automatically).
template <typename I>
struct is_contiguous_iterator : details::contiguous_iterator<I> {};

// std::sentinel_for<S, I> and std::sized_sentinel_for<S, I>.
template <typename S, typename I>
struct is_sentinel_for : details::sentinel_for<S, I> {};

template <typename S, typename I>
struct is_sized_sentinel_for : details::sized_sentinel_for<S, I> {};

// ---------------------------------------------------------------------------
// Variable-template shorthands (core C++14; drop them if a target toolchain is
// configured to reject variable templates).
// ---------------------------------------------------------------------------
template <typename I>
constexpr bool is_iterator_v = is_iterator<I>::value;
template <typename I>
constexpr bool is_input_iterator_v = is_input_iterator<I>::value;
template <typename I, typename T>
constexpr bool is_output_iterator_v = is_output_iterator<I, T>::value;
template <typename I>
constexpr bool is_forward_iterator_v = is_forward_iterator<I>::value;
template <typename I>
constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<I>::value;
template <typename I>
constexpr bool is_random_access_iterator_v = is_random_access_iterator<I>::value;
template <typename I>
constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<I>::value;
template <typename S, typename I>
constexpr bool is_sentinel_for_v = is_sentinel_for<S, I>::value;
template <typename S, typename I>
constexpr bool is_sized_sentinel_for_v = is_sized_sentinel_for<S, I>::value;

} // namespace ztl

#endif // ZTL_ITERATOR_ITERATORCONCEPTS_HPP
