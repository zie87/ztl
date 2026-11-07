// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_FUNCTIONAL_INPLACEFUNCTION_HPP
#define ZTL_FUNCTIONAL_INPLACEFUNCTION_HPP

#include "ztl/functional/invoke.hpp"

#include "ztl/assert.hpp"
#include "ztl/details/function_signature.hpp"
#include "ztl/macros.hpp"
#include "ztl/type_traits.hpp"
#include "ztl/utility.hpp"

#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

// Default in-place buffer size of ztl::inplace_function in bytes. Large enough for a
// lambda capturing a few pointers/references, a function_ref or a bound member call.
#ifndef ZTL_INPLACE_FUNCTION_DEFAULT_CAPACITY
#define ZTL_INPLACE_FUNCTION_DEFAULT_CAPACITY (4U * sizeof(void*))
#endif

namespace ztl {

template <typename SIG_T, std::size_t CAPACITY_V = ZTL_INPLACE_FUNCTION_DEFAULT_CAPACITY,
          std::size_t ALIGN_V = alignof(std::max_align_t)>
class inplace_function;

namespace details {

// -----------------------------------------------------------------------------
// Type-erasure: one constant vtable per stored callable type D. The vtable does
// not depend on the buffer size, so wrappers of different capacity can exchange
// targets directly (and target<T>() can identify D by vtable address).
// -----------------------------------------------------------------------------
template <typename R, typename... ARG_Ts>
struct inplace_vtable {
    R (*call)(void* storage, ARG_Ts&&... args);
    void (*copy)(void* dst, const void* src);
    void (*relocate)(void* dst, void* src); // move-construct into dst, destroy src
    void (*destroy)(void* storage);
};

template <typename D, typename R, typename... ARG_Ts>
struct inplace_target {
    static R call(void* storage, ARG_Ts&&... args) {
        return invoke_r<R>(*static_cast<D*>(storage), std::forward<ARG_Ts>(args)...);
    }

    static void copy(void* dst, const void* src) { ::new (dst) D(*static_cast<const D*>(src)); }

    static void relocate(void* dst, void* src) noexcept {
        D& source = *static_cast<D*>(src);
        ::new (dst) D(std::move(source));
        source.~D();
    }

    static void destroy(void* storage) noexcept { static_cast<D*>(storage)->~D(); }

    static constexpr inplace_vtable<R, ARG_Ts...> vtable = {&call, &copy, &relocate, &destroy};
};

// Empty state: calling it violates the precondition, every other operation is a no-op.
// Using a real vtable instead of nullptr keeps operator() branch-free.
template <typename R, typename... ARG_Ts>
struct inplace_empty_target {
    [[noreturn]] static R call(void*, ARG_Ts&&...) {
        ZTL_EXPECTS(false && "call of an empty inplace_function");
        ::ztl::trap_forever();
    }

    static void copy(void*, const void*) noexcept {}
    static void relocate(void*, void*) noexcept {}
    static void destroy(void*) noexcept {}

    static constexpr inplace_vtable<R, ARG_Ts...> vtable = {&call, &copy, &relocate, &destroy};
};

#if !defined(__cpp_inline_variables) || (__cpp_inline_variables < 201606L)
template <typename D, typename R, typename... ARG_Ts>
constexpr inplace_vtable<R, ARG_Ts...> inplace_target<D, R, ARG_Ts...>::vtable;

template <typename R, typename... ARG_Ts>
constexpr inplace_vtable<R, ARG_Ts...> inplace_empty_target<R, ARG_Ts...>::vtable;
#endif

// -----------------------------------------------------------------------------
// Callables synthesized from ztl::nontype_t (same API as function_ref, but any
// bound arguments are owned by value).
// -----------------------------------------------------------------------------
template <typename F, F f>
struct nontype_invoker {
    template <typename... Ts>
    auto operator()(Ts&&... ts) const -> decltype(::ztl::invoke(f, std::forward<Ts>(ts)...)) {
        return ::ztl::invoke(f, std::forward<Ts>(ts)...);
    }
};

struct bound_args_tag {};

template <typename F, F f, typename... BOUND_Ts>
class bound_nontype_invoker {
public:
    template <typename... Us>
    explicit bound_nontype_invoker(bound_args_tag, Us&&... bound) : bound_(std::forward<Us>(bound)...) {}

    template <typename... Ts>
    auto operator()(Ts&&... ts) -> decltype(::ztl::invoke(f, std::declval<BOUND_Ts&>()..., std::forward<Ts>(ts)...)) {
        return call(std::index_sequence_for<BOUND_Ts...>{}, std::forward<Ts>(ts)...);
    }

private:
    template <std::size_t... I, typename... Ts>
    auto call(std::index_sequence<I...>, Ts&&... ts)
        -> decltype(::ztl::invoke(f, std::declval<BOUND_Ts&>()..., std::forward<Ts>(ts)...)) {
        return ::ztl::invoke(f, std::get<I>(bound_)..., std::forward<Ts>(ts)...);
    }

    std::tuple<BOUND_Ts...> bound_;
};

// -----------------------------------------------------------------------------
// Null detection for std::function compatibility: null function pointers, null
// member pointers and empty inplace_functions (of any signature) yield an empty wrapper.
// -----------------------------------------------------------------------------
template <typename SIG_T, std::size_t CAPACITY_V, std::size_t ALIGN_V>
class inplace_function_base;

template <typename SIG_T, std::size_t CAPACITY_V, std::size_t ALIGN_V>
std::true_type  is_inplace_function_test(const inplace_function_base<SIG_T, CAPACITY_V, ALIGN_V>*);
std::false_type is_inplace_function_test(...);

template <typename T>
struct is_inplace_function : decltype(is_inplace_function_test(std::declval<T*>())){};

template <typename T>
struct is_nullable_callable
    : bool_constant<std::is_pointer<T>::value || std::is_member_pointer<T>::value || is_inplace_function<T>::value> {};

template <typename T>
constexpr bool is_null_callable(const T& callable, std::true_type) noexcept {
    return callable == nullptr;
}

template <typename T>
constexpr bool is_null_callable(const T&, std::false_type) noexcept {
    return false;
}

// -----------------------------------------------------------------------------
// Implementation, shared by R(Args...) and R(Args...) noexcept.
// -----------------------------------------------------------------------------
template <bool Noexcept, typename R, typename... Args, std::size_t CAPACITY_V, std::size_t ALIGN_V>
class inplace_function_base<function_signature<Noexcept, R, Args...>, CAPACITY_V, ALIGN_V> {
    static_assert(CAPACITY_V > 0U, "inplace_function: CAPACITY_V must be greater than zero");
    static_assert(ALIGN_V > 0U && (ALIGN_V & (ALIGN_V - 1U)) == 0U, "inplace_function: ALIGN_V must be a power of two");

    template <typename, std::size_t, std::size_t>
    friend class inplace_function_base;

    using signature_t = function_signature<Noexcept, R, Args...>;
    using vtable_t    = inplace_vtable<R, Args...>;

    template <typename F, typename... BoundArgs>
    using is_invocable = invocable_selector<Noexcept, R, F, BoundArgs..., Args...>;

    // true for every inplace_function (any capacity/alignment) with this signature
    template <std::size_t C, std::size_t A>
    static std::true_type  same_signature_test(const inplace_function_base<signature_t, C, A>*);
    static std::false_type same_signature_test(...);

    template <typename T>
    using is_same_signature = decltype(same_signature_test(std::declval<T*>()));

    template <typename D>
    using is_storable_target = conjunction<
        bool_constant<!is_same_signature<D>::value && !is_nontype<D>::value && !std::is_same<D, std::nullptr_t>::value>,
        std::is_copy_constructible<D>, is_invocable<D&>>;

public:
    using result_type = R;

    static constexpr std::size_t capacity  = CAPACITY_V;
    static constexpr std::size_t alignment = ALIGN_V;

    inplace_function_base() noexcept : vtable_(empty_vtable()) {}

    inplace_function_base(std::nullptr_t) noexcept : vtable_(empty_vtable()) {}

    // Store a copy of an arbitrary callable (function pointers, lambdas, functors,
    // member pointers, function_ref, inplace_function of another signature, ...).
    template <typename F, typename D = typename std::decay<F>::type,
              typename std::enable_if<is_storable_target<D>::value, int>::type = 0>
    inplace_function_base(F&& f) : vtable_(empty_vtable()) {
        if (!is_null_callable<D>(f, is_nullable_callable<D>{})) {
            emplace<D>(std::forward<F>(f));
        }
    }

    // Compile-time function address, mirrors function_ref(nontype_t).
    template <typename F, F f, typename std::enable_if<is_invocable<F>::value, int>::type = 0>
    inplace_function_base(nontype_t<F, f>) noexcept : vtable_(empty_vtable()) {
        emplace<nontype_invoker<F, f>>();
    }

    // Compile-time function address with bound leading arguments owned by value
    // (like std::bind_front). Pass a pointer (or reference_wrapper) to bind by reference.
    template <typename F, F f, typename BoundArg, typename... BoundArgs,
              typename std::enable_if<is_invocable<F, typename std::decay<BoundArg>::type&,
                                                   typename std::decay<BoundArgs>::type&...>::value,
                                      int>::type = 0>
    inplace_function_base(nontype_t<F, f>, BoundArg&& bound, BoundArgs&&... more) : vtable_(empty_vtable()) {
        emplace<
            bound_nontype_invoker<F, f, typename std::decay<BoundArg>::type, typename std::decay<BoundArgs>::type...>>(
            bound_args_tag{}, std::forward<BoundArg>(bound), std::forward<BoundArgs>(more)...);
    }

    inplace_function_base(const inplace_function_base& other) : vtable_(other.vtable_) {
        vtable_->copy(storage(), other.storage());
    }

    inplace_function_base(inplace_function_base&& other) noexcept : vtable_(other.vtable_) {
        other.vtable_ = empty_vtable();
        vtable_->relocate(storage(), other.storage());
    }

    // Adopt the target of a smaller wrapper with the same signature (no double wrapping).
    template <std::size_t C, std::size_t A>
    inplace_function_base(const inplace_function_base<signature_t, C, A>& other) : vtable_(other.vtable_) {
        static_assert(C <= CAPACITY_V, "inplace_function: source capacity exceeds the target capacity");
        static_assert(A <= ALIGN_V, "inplace_function: source alignment exceeds the target alignment");
        vtable_->copy(storage(), other.storage());
    }

    template <std::size_t C, std::size_t A>
    inplace_function_base(inplace_function_base<signature_t, C, A>&& other) noexcept : vtable_(other.vtable_) {
        static_assert(C <= CAPACITY_V, "inplace_function: source capacity exceeds the target capacity");
        static_assert(A <= ALIGN_V, "inplace_function: source alignment exceeds the target alignment");
        other.vtable_ = empty_vtable();
        vtable_->relocate(storage(), other.storage());
    }

    ~inplace_function_base() { vtable_->destroy(storage()); }

    inplace_function_base& operator=(const inplace_function_base& other) {
        if (this != &other) {
            *this = inplace_function_base(other);
        }
        return *this;
    }

    inplace_function_base& operator=(inplace_function_base&& other) noexcept {
        if (this != &other) {
            reset();
            vtable_       = other.vtable_;
            other.vtable_ = empty_vtable();
            vtable_->relocate(storage(), other.storage());
        }
        return *this;
    }

    void swap(inplace_function_base& other) noexcept {
        if (this == &other) {
            return;
        }
        alignas(ALIGN_V) unsigned char tmp[CAPACITY_V];
        vtable_->relocate(tmp, storage());
        other.vtable_->relocate(storage(), other.storage());
        vtable_->relocate(other.storage(), tmp);
        std::swap(vtable_, other.vtable_);
    }

    explicit operator bool() const noexcept { return vtable_ != empty_vtable(); }

    // Precondition: *this is not empty (violations trap via ZTL_EXPECTS).
    R operator()(Args... args) const noexcept(Noexcept) {
        return vtable_->call(storage(), std::forward<Args>(args)...);
    }

    // RTTI-free std::function::target(): identifies the stored type by its vtable.
    template <typename T>
    T* target() noexcept {
        return static_cast<T*>(target_ptr<T>(bool_constant<is_storable_target<T>::value>{}));
    }

    template <typename T>
    const T* target() const noexcept {
        return static_cast<const T*>(const_cast<inplace_function_base*>(this)->template target_ptr<T>(
            bool_constant<is_storable_target<T>::value>{}));
    }

    friend bool operator==(const inplace_function_base& f, std::nullptr_t) noexcept { return !f; }
    friend bool operator==(std::nullptr_t, const inplace_function_base& f) noexcept { return !f; }
    friend bool operator!=(const inplace_function_base& f, std::nullptr_t) noexcept { return static_cast<bool>(f); }
    friend bool operator!=(std::nullptr_t, const inplace_function_base& f) noexcept { return static_cast<bool>(f); }

    friend void swap(inplace_function_base& lhs, inplace_function_base& rhs) noexcept { lhs.swap(rhs); }

protected:
    void reset() noexcept {
        const vtable_t* old = vtable_;
        vtable_             = empty_vtable();
        old->destroy(storage());
    }

private:
    static constexpr const vtable_t* empty_vtable() noexcept { return &inplace_empty_target<R, Args...>::vtable; }

    void* storage() const noexcept { return static_cast<void*>(storage_); }

    template <typename D, typename... CtorArgs>
    void emplace(CtorArgs&&... args) {
        static_assert(sizeof(D) <= CAPACITY_V,
                      "inplace_function: callable does not fit into the buffer, increase CAPACITY_V");
        static_assert(alignof(D) <= ALIGN_V, "inplace_function: callable is over-aligned, increase ALIGN_V");
        static_assert(std::is_copy_constructible<D>::value, "inplace_function: callable must be copy constructible");
        ::new (storage()) D(std::forward<CtorArgs>(args)...);
        vtable_ = &inplace_target<D, R, Args...>::vtable;
    }

    template <typename T>
    void* target_ptr(std::true_type) noexcept {
        return vtable_ == &inplace_target<T, R, Args...>::vtable ? storage() : nullptr;
    }

    template <typename T>
    void* target_ptr(std::false_type) noexcept {
        return nullptr;
    }

    const vtable_t* vtable_;
    alignas(ALIGN_V) mutable unsigned char storage_[CAPACITY_V];
};

#if !defined(__cpp_inline_variables) || (__cpp_inline_variables < 201606L)
template <bool Noexcept, typename R, typename... Args, std::size_t CAPACITY_V, std::size_t ALIGN_V>
constexpr std::size_t inplace_function_base<function_signature<Noexcept, R, Args...>, CAPACITY_V, ALIGN_V>::capacity;

template <bool Noexcept, typename R, typename... Args, std::size_t CAPACITY_V, std::size_t ALIGN_V>
constexpr std::size_t inplace_function_base<function_signature<Noexcept, R, Args...>, CAPACITY_V, ALIGN_V>::alignment;
#endif

} // namespace details

// Owning, copyable, type-erased callable wrapper similar to std::function that never
// allocates: the target is stored in an internal buffer of CAPACITY_V bytes aligned to
// ALIGN_V. Targets that do not fit are rejected at compile time.
//
// Differences to std::function:
//  - calling an empty wrapper is a ZTL_EXPECTS precondition violation (no bad_function_call)
//  - target_type() is not provided (no RTTI); target<T>() works without RTTI
//  - additional nontype_t constructors as in function_ref, binding arguments by value
//  - `inplace_function<R(Args...) noexcept>` with C++17 noexcept function types
//  - copy assignment gives the strong guarantee, the move operations require the
//    target's move constructor not to throw (they are noexcept like std::function's)
template <typename SIG_T, std::size_t CAPACITY_V, std::size_t ALIGN_V>
class inplace_function : public details::inplace_function_base<details::signature_of_t<SIG_T>, CAPACITY_V, ALIGN_V> {
    using base = details::inplace_function_base<details::signature_of_t<SIG_T>, CAPACITY_V, ALIGN_V>;

public:
    using base::base;

    inplace_function() noexcept                              = default;
    inplace_function(const inplace_function&)                = default;
    inplace_function(inplace_function&&) noexcept            = default;
    inplace_function& operator=(const inplace_function&)     = default;
    inplace_function& operator=(inplace_function&&) noexcept = default;

    inplace_function& operator=(std::nullptr_t) noexcept {
        base::reset();
        return *this;
    }

    template <typename F,
              typename std::enable_if<!std::is_same<typename std::decay<F>::type, inplace_function>::value &&
                                          std::is_constructible<inplace_function, F&&>::value,
                                      int>::type = 0>
    inplace_function& operator=(F&& f) {
        return *this = inplace_function(std::forward<F>(f));
    }
};

} // namespace ztl

#endif // ZTL_FUNCTIONAL_INPLACEFUNCTION_HPP
