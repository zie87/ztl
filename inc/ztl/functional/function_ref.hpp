// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_FUNCTIONAL_FUNCTIONREF_HPP
#define ZTL_FUNCTIONAL_FUNCTIONREF_HPP

#include "ztl/functional/invoke.hpp"

#include "ztl/assert.hpp"
#include "ztl/macros.hpp"
#include "ztl/type_traits.hpp"
#include "ztl/utility.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace ztl
{
namespace details
{

union bound_entity_type
{
    void*       obj_ptr;
    const void* const_obj_ptr;
    void (*fn_ptr)();

    bound_entity_type() noexcept : obj_ptr(nullptr) {}

    template<typename T>
    explicit bound_entity_type(
        T* ptr,
        typename std::enable_if<std::is_object<T>::value
                                && !std::is_const<T>::value>::type* = nullptr) noexcept
    : obj_ptr(ptr)
    {
    }

    template<typename T>
    explicit bound_entity_type(
        const T* ptr,
        typename std::enable_if<std::is_object<T>::value>::type* = nullptr) noexcept
    : const_obj_ptr(ptr)
    {
    }

    template<typename T>
    explicit bound_entity_type(
        T* ptr,
        typename std::enable_if<std::is_function<T>::value>::type* = nullptr) noexcept
    : fn_ptr(reinterpret_cast<void (*)()>(ptr))
    {
    }
};

template<typename T>
typename std::enable_if<std::is_const<T>::value, T*>::type
entity_get(bound_entity_type entity) noexcept
{
    return static_cast<T*>(entity.const_obj_ptr);
}

template<typename T>
typename std::enable_if<!std::is_const<T>::value && std::is_object<T>::value, T*>::type
entity_get(bound_entity_type entity) noexcept
{
    return static_cast<T*>(entity.obj_ptr);
}

template<typename T>
typename std::enable_if<std::is_function<T>::value, T*>::type
entity_get(bound_entity_type entity) noexcept
{
    return reinterpret_cast<T*>(entity.fn_ptr);
}

template<bool Noexcept, typename R, typename F, typename... Args>
struct invocable_selector : is_invocable_r<R, F, Args...>
{
};

template<typename R, typename F, typename... Args>
struct invocable_selector<true, R, F, Args...> : is_nothrow_invocable_r<R, F, Args...>
{
};

template<bool Noexcept, typename R, typename... Args>
class function_ref_base
{
    using bound_entity_t = bound_entity_type;
    using thunk_ptr_t    = R (*)(bound_entity_t, Args...);

    template<typename F, typename... BoundArgs>
    struct is_invocable : invocable_selector<Noexcept, R, F, BoundArgs..., Args...>
    {
    };

public:
    // Accept exact function pointers and capture-less lambdas convertible to one.
    function_ref_base(std::nullptr_t) = delete;

    template<bool NX = Noexcept, typename std::enable_if<!NX, int>::type = 0>
    function_ref_base(R (*f)(Args...)) noexcept
    : bound_entity_(f), thunk_ptr_(&function_ptr_thunk<R(Args...)>)
    {
        ZTL_EXPECTS(f != nullptr);
    }

#if ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE
    template<bool NX = Noexcept, typename std::enable_if<NX, int>::type = 0>
    function_ref_base(R (*f)(Args...) noexcept) noexcept
    : bound_entity_(f), thunk_ptr_(&function_ptr_thunk<R(Args...) noexcept>)
    {
        ZTL_EXPECTS(f != nullptr);
    }
#endif

    // Accept function pointers whose return value is convertible to R.
    template<typename F>
    function_ref_base(
        F* f,
        typename std::enable_if<std::is_function<F>::value && !std::is_same<F, R(Args...)>::value
                                && is_invocable<F&>::value>::type* = nullptr) noexcept
    : bound_entity_(f), thunk_ptr_(&function_ptr_thunk<F>)
    {
        ZTL_EXPECTS(f != nullptr);
    }

    // Accept only lvalue callable objects. This avoids dangling references from
    // temporary capturing lambdas/functors.
    template<typename F, typename T = typename std::remove_reference<F>::type>
    function_ref_base(
        F&& f,
        typename std::enable_if<
            !std::is_rvalue_reference<F&&>::value
            && !std::is_base_of<function_ref_base, ::ztl::remove_cvref_t<F>>::value
            && !::ztl::is_nontype<::ztl::remove_cvref_t<F>>::value && !std::is_function<T>::value
            && !std::is_member_pointer<typename std::decay<T>::type>::value
            && is_invocable<T&>::value>::type* = nullptr) noexcept
    : bound_entity_(std::addressof(f)), thunk_ptr_(&object_thunk<T>)
    {
    }

    template<typename F, F f>
    function_ref_base(nontype_t<F, f>,
                      typename std::enable_if<is_invocable<F>::value>::type* = nullptr) noexcept
    : bound_entity_(), thunk_ptr_(&nontype_thunk<F, f>)
    {
    }

    template<typename F, F f, typename U, typename T = typename std::remove_reference<U>::type>
    function_ref_base(
        nontype_t<F, f>,
        U&& obj,
        typename std::enable_if<!std::is_rvalue_reference<U&&>::value
                                && is_invocable<F, T&>::value>::type* = nullptr) noexcept
    : bound_entity_(std::addressof(obj)), thunk_ptr_(&nontype_object_thunk<F, f, T>)
    {
    }

    template<typename F, F f, typename T>
    function_ref_base(nontype_t<F, f>,
                      T* obj,
                      typename std::enable_if<is_invocable<F, T*>::value>::type* = nullptr) noexcept
    : bound_entity_(obj), thunk_ptr_(&nontype_pointer_thunk<F, f, T>)
    {
        ZTL_EXPECTS(obj != nullptr);
    }

    function_ref_base(const function_ref_base&) noexcept            = default;
    function_ref_base& operator=(const function_ref_base&) noexcept = default;

    R operator()(Args... args) const noexcept(Noexcept)
    {
        return thunk_ptr_(bound_entity_, std::forward<Args>(args)...);
    }

private:
    template<typename F>
    static R function_ptr_thunk(bound_entity_t entity, Args... args)
    {
        return invoke_r<R>(*entity_get<F>(entity), std::forward<Args>(args)...);
    }

    template<typename F>
    static R object_thunk(bound_entity_t entity, Args... args)
    {
        F& obj = *entity_get<F>(entity);
        return invoke_r<R>(obj, std::forward<Args>(args)...);
    }

    template<typename F, F f>
    static R nontype_thunk(bound_entity_t, Args... args)
    {
        return invoke_r<R>(f, std::forward<Args>(args)...);
    }

    template<typename F, F f, typename T>
    static R nontype_object_thunk(bound_entity_t entity, Args... args)
    {
        T& obj = *entity_get<T>(entity);
        return invoke_r<R>(f, obj, std::forward<Args>(args)...);
    }

    template<typename F, F f, typename T>
    static R nontype_pointer_thunk(bound_entity_t entity, Args... args)
    {
        T* obj = entity_get<T>(entity);
        return invoke_r<R>(f, obj, std::forward<Args>(args)...);
    }

    bound_entity_t bound_entity_;
    thunk_ptr_t    thunk_ptr_;
};

} // namespace details

template<typename Signature>
class function_ref;

template<typename R, typename... Args>
class function_ref<R(Args...)> : public details::function_ref_base<false, R, Args...>
{
    using base = details::function_ref_base<false, R, Args...>;

public:
    using base::base;

    function_ref(const function_ref&) noexcept            = default;
    function_ref& operator=(const function_ref&) noexcept = default;

    template<typename T>
    typename std::enable_if<!std::is_same<::ztl::remove_cvref_t<T>, function_ref>::value
                                && !std::is_pointer<typename std::decay<T>::type>::value
                                && !::ztl::is_nontype<::ztl::remove_cvref_t<T>>::value,
                            function_ref&>::type
    operator=(T&&)
        = delete;
};

// In standard C++14, noexcept is not part of the function type, so a portable
// function_ref<R(Args...) noexcept> specialization cannot be implemented. If the
// compiler supports C++17 noexcept function types, this branch is enabled.
#if ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE
template<typename R, typename... Args>
class function_ref<R(Args...) noexcept> : public details::function_ref_base<true, R, Args...>
{
    using base = details::function_ref_base<true, R, Args...>;

public:
    using base::base;

    function_ref(const function_ref&) noexcept            = default;
    function_ref& operator=(const function_ref&) noexcept = default;

    template<typename T>
    typename std::enable_if<!std::is_same<::ztl::remove_cvref_t<T>, function_ref>::value
                                && !std::is_pointer<typename std::decay<T>::type>::value
                                && !::ztl::is_nontype<::ztl::remove_cvref_t<T>>::value,
                            function_ref&>::type
    operator=(T&&)
        = delete;
};
#endif

} // namespace ztl

#endif // ZTL_FUNCTIONAL_FUNCTIONREF_HPP
