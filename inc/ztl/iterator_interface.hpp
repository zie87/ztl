/**
 * @file iterator_interface.hpp
 * @brief C++14 / IAR-friendly CRTP adaptation of P2727R4 iterator_interface.
 *
 * This is intentionally not a direct transcription of P2727R4: C++14 has no
 * concepts, deducing-this, if constexpr, or operator<=>.  The public surface and
 * generated operations follow the proposal where this can be done without those
 * language facilities.
 *
 * IAR / embedded C++14 constraints:
 *   - No [[nodiscard]], if constexpr, deduced-this, concepts, or operator<=>
 *   - No std::void_t, std::conjunction, std::enable_if_t
 *   - No dynamic allocation, RTTI, or exceptions
 *
 * Typical direct iterator requirements:
 *
 *   input / forward:
 *       Reference  operator*()  const;
 *       Derived&   operator++();
 *       bool       operator==(Derived const&) const;
 *
 *   bidirectional (above plus):
 *       Derived&   operator--();
 *
 *   random_access:
 *       Reference       operator*()  const;
 *       Derived&        operator+=(DifferenceType);
 *       DifferenceType  operator-(Derived const&) const;
 *       // operator== may be provided explicitly; otherwise it can be
 *       // synthesized from operator-.
 *
 * Adapter support:
 *   A derived iterator may friend ztl::iterator_interface_access and provide
 *   base_reference() overloads.  Missing operator*, ++, --, +=, -, and == are
 *   then synthesized from the adapted base where possible.
 */

#ifndef ZTL_ITERATORINTERFACE_HPP
#define ZTL_ITERATORINTERFACE_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace ztl {

// ===========================================================================
// iterator_interface_access  (P2727R4 §3.1)
// ===========================================================================
struct iterator_interface_access {
    template <typename D>
    static constexpr auto base(D& d) -> decltype(d.base_reference()) {
        return d.base_reference();
    }

    template <typename D>
    static constexpr auto base(D const& d) -> decltype(d.base_reference()) {
        return d.base_reference();
    }
};

// ===========================================================================
// proxy_arrow_result  (P2727R4 §3.2)
// ===========================================================================
template <typename T>
class proxy_arrow_result {
    static_assert(std::is_object<T>::value, "ztl::proxy_arrow_result<T> requires an object type T");

public:
    constexpr explicit proxy_arrow_result(T const& v) : val_(v) {}
    constexpr explicit proxy_arrow_result(T&& v) : val_(static_cast<T&&>(v)) {}

    constexpr T const* operator->() const { return &val_; }
    constexpr T*       operator->() { return &val_; }

private:
    T val_;
};

namespace details {

// -----------------------------------------------------------------------
// C++14 utilities
// -----------------------------------------------------------------------
template <typename...>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

template <typename T>
T* addressof(T& value) {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<char const volatile&>(value)));
}

template <typename Pointer, typename Reference, bool IsPointer>
struct make_iterator_pointer_impl;

template <typename Pointer, typename Reference>
struct make_iterator_pointer_impl<Pointer, Reference, true> {
    template <typename T>
    static Pointer call(T&& value) {
        return details::addressof(value);
    }
};

template <typename Pointer, typename Reference>
struct make_iterator_pointer_impl<Pointer, Reference, false> {
    template <typename T>
    static Pointer call(T&& value) {
        return Pointer(std::forward<T>(value));
    }
};

template <typename Pointer, typename Reference, typename T>
Pointer make_iterator_pointer(T&& value) {
    return make_iterator_pointer_impl<Pointer, Reference, std::is_pointer<Pointer>::value>::call(
        std::forward<T>(value));
}

// -----------------------------------------------------------------------
// Detection helpers.  These are deliberately simple C++14 SFINAE traits.
// -----------------------------------------------------------------------
template <typename T, typename = void>
struct has_base : std::false_type {};

template <typename T>
struct has_base<T, void_t<decltype(iterator_interface_access::base(std::declval<T&>()))>> : std::true_type {};

template <typename T, typename = void>
struct has_const_base : std::false_type {};

template <typename T>
struct has_const_base<T, void_t<decltype(iterator_interface_access::base(std::declval<T const&>()))>> : std::true_type {
};

template <typename T, typename = void>
struct has_base_preincrement : std::false_type {};

template <typename T>
struct has_base_preincrement<T, void_t<decltype(++iterator_interface_access::base(std::declval<T&>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_base_predecrement : std::false_type {};

template <typename T>
struct has_base_predecrement<T, void_t<decltype(--iterator_interface_access::base(std::declval<T&>()))>>
    : std::true_type {};

template <typename T, typename DiffT, typename = void>
struct has_base_plus_equal : std::false_type {};

template <typename T, typename DiffT>
struct has_base_plus_equal<
    T, DiffT, void_t<decltype(iterator_interface_access::base(std::declval<T&>()) += std::declval<DiffT>())>>
    : std::true_type {};

template <typename T, typename = void>
struct has_base_subtract : std::false_type {};

template <typename T>
struct has_base_subtract<T, void_t<decltype(iterator_interface_access::base(std::declval<T const&>()) -
                                            iterator_interface_access::base(std::declval<T const&>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_base_equal : std::false_type {};

template <typename T>
struct has_base_equal<T, void_t<decltype(iterator_interface_access::base(std::declval<T const&>()) ==
                                         iterator_interface_access::base(std::declval<T const&>()))>> : std::true_type {
};

template <typename T, typename = void>
struct has_member_equal : std::false_type {};

template <typename T>
struct has_member_equal<T, void_t<decltype(std::declval<T const&>().operator==(std::declval<T const&>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_member_subtract : std::false_type {};

template <typename T>
struct has_member_subtract<T, void_t<decltype(std::declval<T const&>().operator-(std::declval<T const&>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct has_iter_subtract : std::false_type {};

template <typename T>
struct has_iter_subtract<T, void_t<decltype(std::declval<T const&>() - std::declval<T const&>())>> : std::true_type {};

// -----------------------------------------------------------------------
// Iterator concept normalization.  P2727 uses derived_from; with C++14 tags
// we normalize custom derived tags to one of the legacy operation groups.
// -----------------------------------------------------------------------
template <typename IteratorConcept>
struct ops_tag {
private:
    typedef typename std::conditional<
        std::is_base_of<std::random_access_iterator_tag, IteratorConcept>::value, std::random_access_iterator_tag,
        typename std::conditional<
            std::is_base_of<std::bidirectional_iterator_tag, IteratorConcept>::value, std::bidirectional_iterator_tag,
            typename std::conditional<
                std::is_base_of<std::forward_iterator_tag, IteratorConcept>::value, std::forward_iterator_tag,
                typename std::conditional<std::is_same<IteratorConcept, std::output_iterator_tag>::value,
                                          std::output_iterator_tag, std::input_iterator_tag>::type>::type>::type>::type
        tag;

public:
    typedef tag type;
};

template <typename IteratorConcept>
struct supported_concept {
    enum {
        value = std::is_same<IteratorConcept, std::output_iterator_tag>::value ||
                std::is_base_of<std::input_iterator_tag, IteratorConcept>::value ||
                std::is_base_of<std::forward_iterator_tag, IteratorConcept>::value ||
                std::is_base_of<std::bidirectional_iterator_tag, IteratorConcept>::value ||
                std::is_base_of<std::random_access_iterator_tag, IteratorConcept>::value
    };
};

// -----------------------------------------------------------------------
// iterator_category is present iff IteratorConcept is forward-or-better.
// When Reference is not a language reference, it is downgraded to input.
// -----------------------------------------------------------------------
template <typename IteratorConcept, typename Reference,
          bool HasCategory = std::is_base_of<std::forward_iterator_tag, IteratorConcept>::value>
struct iterator_category_base {};

template <typename IteratorConcept, typename Reference>
struct iterator_category_base<IteratorConcept, Reference, true> {
private:
    typedef typename std::conditional<
        !std::is_reference<Reference>::value, std::input_iterator_tag,
        typename std::conditional<
            std::is_base_of<std::random_access_iterator_tag, IteratorConcept>::value, std::random_access_iterator_tag,
            typename std::conditional<std::is_base_of<std::bidirectional_iterator_tag, IteratorConcept>::value,
                                      std::bidirectional_iterator_tag, std::forward_iterator_tag>::type>::type>::type
        category;

public:
    typedef category iterator_category;
};

// -----------------------------------------------------------------------
// iter_ops: single mixin per normalized category tag.
// -----------------------------------------------------------------------
template <typename D, typename DiffT, typename Ref, typename Tag>
struct iter_ops;

// --- output_iterator_tag -----------------------------------------------
template <typename D, typename DiffT, typename Ref>
struct iter_ops<D, DiffT, Ref, std::output_iterator_tag> {};

// --- input_iterator_tag ------------------------------------------------
template <typename D, typename DiffT, typename Ref>
struct iter_ops<D, DiffT, Ref, std::input_iterator_tag> {
    template <typename T = D>
    constexpr typename std::enable_if<has_base_preincrement<T>::value, D&>::type operator++() {
        ++iterator_interface_access::base(static_cast<D&>(*this));
        return static_cast<D&>(*this);
    }

    // P2727R4: postfix ++ returns void for input iterators.
    friend constexpr void operator++(D& self, int) { ++self; }

    template <typename X = D>
    friend constexpr
        typename std::enable_if<std::is_same<X, D>::value && !has_member_equal<X>::value && has_base_equal<X>::value,
                                bool>::type
        operator==(X const& a, X const& b) {
        return iterator_interface_access::base(a) == iterator_interface_access::base(b);
    }

    friend constexpr bool operator!=(D const& a, D const& b) { return !(a == b); }
};

// --- forward_iterator_tag ----------------------------------------------
template <typename D, typename DiffT, typename Ref>
struct iter_ops<D, DiffT, Ref, std::forward_iterator_tag> {
    template <typename T = D>
    constexpr typename std::enable_if<has_base_preincrement<T>::value, D&>::type operator++() {
        ++iterator_interface_access::base(static_cast<D&>(*this));
        return static_cast<D&>(*this);
    }

    friend constexpr D operator++(D& self, int) {
        D tmp = self;
        ++self;
        return tmp;
    }

    template <typename X = D>
    friend constexpr
        typename std::enable_if<std::is_same<X, D>::value && !has_member_equal<X>::value && has_base_equal<X>::value,
                                bool>::type
        operator==(X const& a, X const& b) {
        return iterator_interface_access::base(a) == iterator_interface_access::base(b);
    }

    friend constexpr bool operator!=(D const& a, D const& b) { return !(a == b); }
};

// --- bidirectional_iterator_tag ----------------------------------------
template <typename D, typename DiffT, typename Ref>
struct iter_ops<D, DiffT, Ref, std::bidirectional_iterator_tag> {
    template <typename T = D>
    constexpr typename std::enable_if<has_base_preincrement<T>::value, D&>::type operator++() {
        ++iterator_interface_access::base(static_cast<D&>(*this));
        return static_cast<D&>(*this);
    }

    template <typename T = D>
    constexpr typename std::enable_if<has_base_predecrement<T>::value, D&>::type operator--() {
        --iterator_interface_access::base(static_cast<D&>(*this));
        return static_cast<D&>(*this);
    }

    friend constexpr D operator++(D& self, int) {
        D tmp = self;
        ++self;
        return tmp;
    }

    friend constexpr D operator--(D& self, int) {
        D tmp = self;
        --self;
        return tmp;
    }

    template <typename X = D>
    friend constexpr
        typename std::enable_if<std::is_same<X, D>::value && !has_member_equal<X>::value && has_base_equal<X>::value,
                                bool>::type
        operator==(X const& a, X const& b) {
        return iterator_interface_access::base(a) == iterator_interface_access::base(b);
    }

    friend constexpr bool operator!=(D const& a, D const& b) { return !(a == b); }
};

// --- random_access_iterator_tag ----------------------------------------
template <typename D, typename DiffT, typename Ref>
struct iter_ops<D, DiffT, Ref, std::random_access_iterator_tag> {
    // += from adapted base, when present.  A user-provided Derived::operator+=
    // hides this member.
    template <typename T = D>
    constexpr typename std::enable_if<has_base_plus_equal<T, DiffT>::value, D&>::type operator+=(DiffT n) {
        iterator_interface_access::base(static_cast<D&>(*this)) += n;
        return static_cast<D&>(*this);
    }

    // prefix ++ / -- synthesized from +=
    constexpr D& operator++() {
        D& self = static_cast<D&>(*this);
        self += DiffT(1);
        return self;
    }

    constexpr D& operator--() {
        D& self = static_cast<D&>(*this);
        self += DiffT(-1);
        return self;
    }

    friend constexpr D operator++(D& self, int) {
        D tmp = self;
        ++self;
        return tmp;
    }

    friend constexpr D operator--(D& self, int) {
        D tmp = self;
        --self;
        return tmp;
    }

    // -= from +=
    constexpr D& operator-=(DiffT n) {
        static_cast<D&>(*this) += static_cast<DiffT>(-n);
        return static_cast<D&>(*this);
    }

    // [] from + and *; returns exactly whatever * returns.
    // Note: uses explicit return type instead of decltype(auto) for IAR C++14 compatibility.
    constexpr Ref operator[](DiffT n) const {
        D tmp = static_cast<D const&>(*this);
        tmp   = tmp + n;
        return *tmp;
    }

    friend constexpr D operator+(D it, DiffT n) {
        it += n;
        return it;
    }

    friend constexpr D operator+(DiffT n, D it) {
        it += n;
        return it;
    }

    friend constexpr D operator-(D it, DiffT n) {
        it += static_cast<DiffT>(-n);
        return it;
    }

    // iterator - iterator from adapted base, when the derived type does not
    // provide a member operator-.
    template <typename X = D>
    friend constexpr typename std::enable_if<
        std::is_same<X, D>::value && !has_member_subtract<X>::value && has_base_subtract<X>::value, DiffT>::type
    operator-(X const& a, X const& b) {
        return static_cast<DiffT>(iterator_interface_access::base(a) - iterator_interface_access::base(b));
    }

    // Equality from adapted base, otherwise from subtraction.  This is removed
    // when the derived type already has a member operator==.
    template <typename X = D>
    friend constexpr
        typename std::enable_if<std::is_same<X, D>::value && !has_member_equal<X>::value && has_base_equal<X>::value,
                                bool>::type
        operator==(X const& a, X const& b) {
        return iterator_interface_access::base(a) == iterator_interface_access::base(b);
    }

    template <typename X = D>
    friend constexpr typename std::enable_if<std::is_same<X, D>::value && !has_member_equal<X>::value &&
                                                 !has_base_equal<X>::value && has_iter_subtract<X>::value,
                                             bool>::type
    operator==(X const& a, X const& b) {
        return (a - b) == DiffT(0);
    }

    friend constexpr bool operator!=(D const& a, D const& b) { return !(a == b); }

    friend constexpr bool operator<(D const& a, D const& b) { return (a - b) < DiffT(0); }

    friend constexpr bool operator>(D const& a, D const& b) { return b < a; }

    friend constexpr bool operator<=(D const& a, D const& b) { return !(b < a); }

    friend constexpr bool operator>=(D const& a, D const& b) { return !(a < b); }
};

// -----------------------------------------------------------------------
// arrow mixin.  P2727 enables operator-> only when pointer != void and
// reference is a language reference.  proxy_arrow_result is therefore exposed
// as the pointer type for proxy iterators but is not used by operator-> unless
// the Reference template argument is itself a reference.
// -----------------------------------------------------------------------
template <typename D, typename Ref, typename Ptr, bool Enabled>
struct arrow_mixin {};

template <typename D, typename Ref, typename Ptr>
struct arrow_mixin<D, Ref, Ptr, true> {
    Ptr operator->() const { return details::make_iterator_pointer<Ptr, Ref>(*static_cast<D const&>(*this)); }
};

} // namespace details

// ===========================================================================
// iterator_interface
// ===========================================================================
template <typename Derived, typename IteratorConcept, typename ValueType, typename Reference = ValueType&,
          typename Pointer = ValueType*, typename DifferenceType = std::ptrdiff_t>
class iterator_interface
    : public details::iterator_category_base<IteratorConcept, Reference>,
      public details::iter_ops<Derived, DifferenceType, Reference, typename details::ops_tag<IteratorConcept>::type>,
      public details::arrow_mixin<
          Derived, Reference,
          typename std::conditional<std::is_same<IteratorConcept, std::output_iterator_tag>::value, void,
                                    Pointer>::type,
          !std::is_same<typename std::conditional<std::is_same<IteratorConcept, std::output_iterator_tag>::value, void,
                                                  Pointer>::type,
                        void>::value &&
              std::is_reference<Reference>::value> {
    typedef
        typename std::conditional<std::is_same<IteratorConcept, std::output_iterator_tag>::value, void, Pointer>::type
            pointer_type;

public:
    static_assert(details::supported_concept<IteratorConcept>::value,
                  "ztl::iterator_interface: unsupported iterator concept tag");

    typedef IteratorConcept                          iterator_concept;
    typedef typename std::remove_cv<ValueType>::type value_type;
    typedef Reference                                reference;
    typedef pointer_type                             pointer;
    typedef DifferenceType                           difference_type;

    // operator* from adapted base, when present.  A user-provided
    // Derived::operator* hides these members.
    template <typename T = Derived>
    constexpr auto operator*() -> decltype(*iterator_interface_access::base(std::declval<T&>())) {
        return *iterator_interface_access::base(static_cast<Derived&>(*this));
    }

    template <typename T = Derived>
    constexpr auto operator*() const -> decltype(*iterator_interface_access::base(std::declval<T const&>())) {
        return *iterator_interface_access::base(static_cast<Derived const&>(*this));
    }

protected:
    iterator_interface()                                     = default;
    ~iterator_interface()                                    = default;
    iterator_interface(iterator_interface const&)            = default;
    iterator_interface& operator=(iterator_interface const&) = default;
};

// ===========================================================================
// proxy_iterator_interface  (P2727R4 §3.6)
// ===========================================================================
template <typename Derived, typename IteratorConcept, typename ValueType, typename Reference = ValueType,
          typename DifferenceType = std::ptrdiff_t>
using proxy_iterator_interface =
    iterator_interface<Derived, IteratorConcept, ValueType, Reference, proxy_arrow_result<Reference>, DifferenceType>;

} // namespace ztl

#endif // ZTL_ITERATORINTERFACE_HPP
