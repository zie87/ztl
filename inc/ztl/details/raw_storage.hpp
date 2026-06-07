// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_DETAILS_RAWSTORAGE_HPP
#define ZTL_DETAILS_RAWSTORAGE_HPP

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace ztl { namespace details {

template <typename T, std::size_t SLOT_CNT>
class raw_storage final {
    static_assert(SLOT_CNT > 0U, "raw_storage: SLOT_CNT must be greater than zero");
    static_assert(std::is_object<T>::value, "raw_storage: T must be an object type");
    static_assert(!std::is_const<T>::value, "raw_storage: T must not be const");
    static_assert(std::is_destructible<T>::value, "raw_storage: T must be destructible");

public:
    typedef T           value_type;
    typedef std::size_t size_type;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T*          pointer;
    typedef const T*    const_pointer;

    raw_storage() noexcept = default;

    raw_storage(const raw_storage&)            = delete;
    raw_storage& operator=(const raw_storage&) = delete;

    pointer ptr(size_type index) noexcept { return reinterpret_cast<pointer>(&m_storage[index * sizeof(T)]); }

    const_pointer ptr(size_type index) const noexcept {
        return reinterpret_cast<const_pointer>(&m_storage[index * sizeof(T)]);
    }

    reference ref(size_type index) noexcept { return *ptr(index); }

    const_reference ref(size_type index) const noexcept { return *ptr(index); }

    template <typename... ARG_Ts>
    void construct(size_type index, ARG_Ts&&... args) noexcept(std::is_nothrow_constructible<T, ARG_Ts&&...>::value) {
        static_assert(std::is_constructible<T, ARG_Ts&&...>::value,
                      "raw_storage::construct(): T must be constructible from ARG_Ts...");
        ::new (static_cast<void*>(ptr(index))) T(std::forward<ARG_Ts>(args)...);
    }

    void destroy(size_type index) noexcept { ptr(index)->~T(); }

private:
    // Using alignas + unsigned char instead of the deprecated std::aligned_storage (C++23).
    alignas(T) unsigned char m_storage[SLOT_CNT * sizeof(T)];
};

}} // namespace ztl::details

#endif // ZTL_DETAILS_RAWSTORAGE_HPP
