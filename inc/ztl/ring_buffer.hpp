// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_RINGBUFFER_HPP
#define ZTL_RINGBUFFER_HPP

#include "ztl/iterator_interface.hpp"
#include "ztl/details/raw_storage.hpp"
#include "ztl/type_traits.hpp"
#include "ztl/assert.hpp"

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <iterator>
#include <limits>

namespace ztl {

// ════════════════════════════════════════════════════════════════════════
//  Pop strategies
// ════════════════════════════════════════════════════════════════════════

/// @brief Does nothing to the popped element. pop_front()/pop_back() return void.
///        The ring buffer destroys the element after the popper is called.
template <typename T>
struct null_popper {
    void operator()(T& /*unused*/) const noexcept {}
};

/// @brief Moves the popped element out. pop_front()/pop_back() return T.
template <typename T>
struct default_popper {
    T operator()(T& t) const { return std::move(t); }
};

/// @brief API-compatible popper that stores a user supplied value.
///
/// With raw storage the popped slot is destroyed immediately after the popper
/// runs, so there is no persistent slot to refill.
///
/// @note The stored @c copy_ member is *not* used in the raw-storage variant.
///       It is retained solely for source/ABI compatibility with the
///       pre-constructed-storage ring buffer variant. If you are exclusively
///       using the raw-storage ring buffer, prefer @c default_popper instead
///       to avoid wasting SRAM.
template <typename T>
struct copy_popper {
    explicit copy_popper(T t) : copy_(std::move(t)) {}

    T operator()(T& t) const { return std::move(t); }

    T copy_; ///< Unused in raw-storage variant. Kept for API compatibility.
};

// ════════════════════════════════════════════════════════════════════════
//  Push overflow strategies
// ════════════════════════════════════════════════════════════════════════

/// @brief Drops the oldest element.
///
/// @par Overflow policy contract
/// A conforming overflow policy must provide:
/// @code
///   static bool on_overflow(std::size_t& tail, std::size_t slots) noexcept;
/// @endcode
/// The function must either:
///   - advance @c tail by exactly one slot: `tail = (tail + 1U) % slots;`
///     and return @c true (element will be overwritten), or
///   - leave @c tail unchanged and return @c false (push is rejected).
///
/// Advancing @c tail by more than one slot, or any other mutation, is
/// undefined behaviour and will corrupt the ring buffer.
struct OverwritePolicy {
    static bool on_overflow(std::size_t& tail, std::size_t slots) noexcept {
        tail = (tail + 1U) % slots;
        return true;
    }
};

/// @brief Silently rejects the push. Buffer and data stay unchanged.
struct DiscardPolicy {
    static bool on_overflow(std::size_t& /*tail*/, std::size_t /*slots*/) noexcept { return false; }
};

namespace details {

template <typename Policy, typename = void>
struct has_overflow_policy : std::false_type {};

template <typename Policy>
struct has_overflow_policy<
    Policy, ::ztl::void_t<decltype(Policy::on_overflow(std::declval<std::size_t&>(), std::declval<std::size_t>()))>>
    : std::integral_constant<
          bool,
          std::is_convertible<decltype(Policy::on_overflow(std::declval<std::size_t&>(), std::declval<std::size_t>())),
                              bool>::value &&
              noexcept(Policy::on_overflow(std::declval<std::size_t&>(), std::declval<std::size_t>()))> {};

template <typename Popper, typename Element, typename = void>
struct is_popper : std::false_type {};

template <typename Popper, typename Element>
struct is_popper<Popper, Element, ::ztl::void_t<decltype(std::declval<Popper&>()(std::declval<Element&>()))>>
    : std::true_type {};

} // namespace details

// ════════════════════════════════════════════════════════════════════════
//  ring_buffer_base
// ════════════════════════════════════════════════════════════════════════

/// @brief Fixed-capacity owning ring-buffer storage and algorithms.
///
/// This class only depends on @c T and @c Capacity. Operations that need extra
/// behaviour receive it at the function boundary:
///   - overflowing push operations take the overflow policy as a function
///     template parameter;
///   - pop operations take the popper as a call parameter.
///
/// Only logically stored elements are constructed. Empty slots contain raw
/// memory only. This removes the default-constructibility and assignment
/// requirements of a pre-constructed `T storage[N]` implementation.
///
/// @note **Sentinel slot overhead.** Internally allocates `Capacity + 1` slots
///       to distinguish full from empty via head/tail index comparison. For
///       very small capacities (e.g. `Capacity == 1`) this is a 50% overhead.
///       On memory-constrained targets consider whether that trade-off is
///       acceptable.
///
/// @warning **Not interrupt-safe / not thread-safe.** All operations (push,
///          pop, iteration, clear) read and write shared indices (`m_head`,
///          `m_tail`) without synchronisation. If the ring buffer is accessed
///          from both an ISR and the main loop, the caller must provide
///          explicit mutual exclusion (e.g. disable interrupts around each
///          access, use a critical section, or use a separate lock-free SPSC
///          queue designed for that purpose).
///
/// @tparam T        Element type. Does not need to be default-constructible.
/// @tparam Capacity Maximum number of elements. Must be >= 1.
template <typename T, std::size_t Capacity>
class ring_buffer_base final {
    static_assert(Capacity >= 1U, "ring_buffer_base: Capacity must be at least 1");
    static_assert(Capacity < (std::numeric_limits<std::size_t>::max)(),
                  "ring_buffer_base: Capacity + 1 would overflow size_type");
    static_assert(Capacity <= static_cast<std::size_t>((std::numeric_limits<std::ptrdiff_t>::max)()),
                  "ring_buffer_base: Capacity must fit into difference_type");
    static_assert(std::is_object<T>::value, "ring_buffer_base: T must be an object type");
    static_assert(!std::is_const<T>::value, "ring_buffer_base: T must not be const");
    static_assert(std::is_destructible<T>::value, "ring_buffer_base: T must be destructible");

    static constexpr std::size_t kSlots = Capacity + 1U;

public:
    using type            = ring_buffer_base;
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;

    template <bool>
    class iterator_impl;

    using iterator       = iterator_impl<false>;
    using const_iterator = iterator_impl<true>;

    ring_buffer_base() noexcept : m_head{0U}, m_tail{0U}, m_storage{} {}

    ring_buffer_base(const ring_buffer_base& rhs) : m_head{0U}, m_tail{0U}, m_storage{} { copy_construct_from(rhs); }

    ring_buffer_base& operator=(const ring_buffer_base& rhs) {
        static_assert(std::is_copy_constructible<T>::value,
                      "ring_buffer_base::operator=(const ring_buffer_base&): T must be copy-constructible");

        if (this != &rhs) {
            clear();
            copy_construct_from(rhs);
        }
        return *this;
    }

    ring_buffer_base(ring_buffer_base&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_head{0U}, m_tail{0U}, m_storage{} {
        move_construct_from(rhs);
        rhs.clear();
    }

    ring_buffer_base& operator=(ring_buffer_base&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_move_constructible<T>::value,
                      "ring_buffer_base::operator=(ring_buffer_base&&): T must be move-constructible");

        if (this != &rhs) {
            clear();
            move_construct_from(rhs);
            rhs.clear();
        }
        return *this;
    }

    ~ring_buffer_base() noexcept { clear(); }

    bool empty() const noexcept { return m_head == m_tail; }

    bool full() const noexcept { return next_idx(m_head) == m_tail; }

    size_type size() const noexcept {
        const size_type h = m_head;
        const size_type t = m_tail;
        return (h >= t) ? (h - t) : (kSlots - t + h);
    }

    static constexpr size_type capacity() noexcept { return Capacity; }

    /// @pre !empty()
    reference front() noexcept {
        ZTL_EXPECTS(!empty());
        return storage_ref(m_tail);
    }
    const_reference front() const noexcept {
        ZTL_EXPECTS(!empty());
        return storage_ref(m_tail);
    }

    /// @pre !empty()
    reference back() noexcept {
        ZTL_EXPECTS(!empty());
        return storage_ref(prev_idx(m_head));
    }
    const_reference back() const noexcept {
        ZTL_EXPECTS(!empty());
        return storage_ref(prev_idx(m_head));
    }

    /// @brief Indexed access relative to front. 0 is the oldest element.
    /// @pre index < size()
    reference operator[](size_type index) noexcept {
        ZTL_EXPECTS(index < size());
        return storage_ref(physical_index(index));
    }
    const_reference operator[](size_type index) const noexcept {
        ZTL_EXPECTS(index < size());
        return storage_ref(physical_index(index));
    }

    /// @brief Push a copy at the back. Overflow is handled by @c OverflowPolicy.
    template <typename OverflowPolicy>
    void push_back(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value,
                      "ring_buffer_base::push_back(const T&): T must be copy-constructible");
        push_context context;
        if (!prepare_push<OverflowPolicy>(context)) {
            return;
        }

        construct_at(context.head, value);
        commit_push(context);
    }

    /// @brief Push a moved value at the back. Overflow is handled by @c OverflowPolicy.
    template <typename OverflowPolicy>
    void push_back(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_move_constructible<T>::value,
                      "ring_buffer_base::push_back(T&&): T must be move-constructible");
        push_context context;
        if (!prepare_push<OverflowPolicy>(context)) {
            return;
        }

        construct_at(context.head, std::move(value));
        commit_push(context);
    }

    /// @brief Construct an element directly in the back slot.
    template <typename OverflowPolicy, typename... Args>
    void emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
        static_assert(std::is_constructible<T, Args&&...>::value,
                      "ring_buffer_base::emplace_back(): T must be constructible from Args...");
        push_context context;
        if (!prepare_push<OverflowPolicy>(context)) {
            return;
        }

        construct_at(context.head, std::forward<Args>(args)...);
        commit_push(context);
    }

    /// @brief Push a copy only when space is available. Never overwrites.
    bool try_push_back(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value,
                      "ring_buffer_base::try_push_back(const T&): T must be copy-constructible");
        if (full()) {
            return false;
        }
        construct_at(m_head, value);
        m_head = next_idx(m_head);
        return true;
    }

    /// @brief Push a moved value only when space is available. Never overwrites.
    bool try_push_back(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_move_constructible<T>::value,
                      "ring_buffer_base::try_push_back(T&&): T must be move-constructible");
        if (full()) {
            return false;
        }
        construct_at(m_head, std::move(value));
        m_head = next_idx(m_head);
        return true;
    }

    /// @brief Construct an element directly in the back slot only when space is available.
    template <typename... Args>
    bool try_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
        static_assert(std::is_constructible<T, Args&&...>::value,
                      "ring_buffer_base::try_emplace_back(): T must be constructible from Args...");
        if (full()) {
            return false;
        }
        construct_at(m_head, std::forward<Args>(args)...);
        m_head = next_idx(m_head);
        return true;
    }

    /// @brief Remove the oldest element through @c popper, then destroy the slot.
    /// @pre !empty()
    template <typename Popper>
    auto pop_front(Popper& popper) -> decltype(std::declval<Popper&>()(std::declval<T&>())) {
        static_assert(details::is_popper<Popper, T>::value,
                      "ring_buffer_base::pop_front(): popper must be callable as popper(T&)");
        ZTL_EXPECTS(!empty());

        typedef decltype(std::declval<Popper&>()(std::declval<T&>())) result_type;
        return pop_front_impl<Popper, result_type>(popper);
    }

    /// @brief Remove the newest element through @c popper, then destroy the slot.
    /// @pre !empty()
    template <typename Popper>
    auto pop_back(Popper& popper) -> decltype(std::declval<Popper&>()(std::declval<T&>())) {
        static_assert(details::is_popper<Popper, T>::value,
                      "ring_buffer_base::pop_back(): popper must be callable as popper(T&)");
        ZTL_EXPECTS(!empty());

        typedef decltype(std::declval<Popper&>()(std::declval<T&>())) result_type;
        return pop_back_impl<Popper, result_type>(popper);
    }

    /// @brief Destroy all live elements and reset the indices to zero.
    void clear() noexcept {
        while (!empty()) {
            destroy_at(m_tail);
            m_tail = next_idx(m_tail);
        }
        m_head = 0U;
        m_tail = 0U;
    }

    /// @brief Swap contents. O(N), requires nothrow move construction of T.
    void swap(ring_buffer_base& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_nothrow_move_constructible<T>::value,
                      "ring_buffer_base::swap(): T must be nothrow-move-constructible. "
                      "A throwing move would leave both buffers in a corrupt state.");

        if (this == &rhs) {
            return;
        }

        raw_storage_type temp;
        size_type        temp_count = 0U;

        const size_type lhs_count = size();
        for (size_type i = 0U; i < lhs_count; ++i) {
            temp.construct(i, std::move((*this)[i]));
            ++temp_count;
        }
        clear();

        const size_type rhs_count = rhs.size();
        for (size_type i = 0U; i < rhs_count; ++i) {
            construct_at(m_head, std::move(rhs[i]));
            m_head = next_idx(m_head);
        }
        rhs.clear();

        for (size_type i = 0U; i < temp_count; ++i) {
            rhs.construct_at(rhs.m_head, std::move(temp.ref(i)));
            rhs.m_head = rhs.next_idx(rhs.m_head);
            temp.destroy(i);
        }
    }

    // ── iterators ──────────────────────────────────────────────────────

    template <bool IsConst>
    class iterator_impl final
        : public ztl::iterator_interface<iterator_impl<IsConst>, std::random_access_iterator_tag, T,
                                         typename std::conditional<IsConst, const T&, T&>::type,
                                         typename std::conditional<IsConst, const T*, T*>::type, std::ptrdiff_t> {
        using ring_ptr = typename std::conditional<IsConst, const ring_buffer_base*, ring_buffer_base*>::type;

    public:
        using base_type =
            ztl::iterator_interface<iterator_impl<IsConst>, std::random_access_iterator_tag, T,
                                    typename std::conditional<IsConst, const T&, T&>::type,
                                    typename std::conditional<IsConst, const T*, T*>::type, std::ptrdiff_t>;

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = typename std::conditional<IsConst, const T*, T*>::type;
        using reference         = typename std::conditional<IsConst, const T&, T&>::type;

        iterator_impl() noexcept : m_ring{nullptr}, m_idx{0} {}

        reference operator*() const noexcept {
            ZTL_EXPECTS(m_ring != nullptr);
            ZTL_EXPECTS(m_idx >= 0);
            ZTL_EXPECTS(static_cast<size_type>(m_idx) < m_ring->size());
            return (*m_ring)[static_cast<size_type>(m_idx)];
        }

        iterator_impl& operator+=(difference_type n) noexcept {
            m_idx += n;
            return *this;
        }

        difference_type operator-(const iterator_impl& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return m_idx - rhs.m_idx;
        }

        template <bool C>
        difference_type operator-(const iterator_impl<C>& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return m_idx - rhs.m_idx;
        }

        template <bool C>
        bool operator==(const iterator_impl<C>& rhs) const noexcept {
            return (m_ring == rhs.m_ring) && (m_idx == rhs.m_idx);
        }

        template <bool C>
        bool operator!=(const iterator_impl<C>& rhs) const noexcept {
            return !(*this == rhs);
        }

        template <bool C>
        bool operator<(const iterator_impl<C>& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return m_idx < rhs.m_idx;
        }

        template <bool C>
        bool operator>(const iterator_impl<C>& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return rhs < *this;
        }

        template <bool C>
        bool operator<=(const iterator_impl<C>& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return !(rhs < *this);
        }

        template <bool C>
        bool operator>=(const iterator_impl<C>& rhs) const noexcept {
            ZTL_EXPECTS(m_ring == rhs.m_ring);
            return !(*this < rhs);
        }

        template <bool B = IsConst, typename std::enable_if<!B, int>::type = 0>
        operator iterator_impl<true>() const noexcept {
            return iterator_impl<true>{m_ring, m_idx};
        }

    private:
        friend class ring_buffer_base;
        template <bool>
        friend class iterator_impl;

        iterator_impl(ring_ptr ring, difference_type idx) noexcept : m_ring{ring}, m_idx{idx} {}

        ring_ptr        m_ring;
        difference_type m_idx;
    };

    iterator       begin() noexcept { return iterator{this, 0}; }
    iterator       end() noexcept { return iterator{this, static_cast<difference_type>(size())}; }
    const_iterator begin() const noexcept { return const_iterator{this, 0}; }
    const_iterator end() const noexcept { return const_iterator{this, static_cast<difference_type>(size())}; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

private:
    typedef details::raw_storage<T, kSlots> raw_storage_type;

    static constexpr size_type next_idx(size_type i) noexcept { return (i + 1U == kSlots) ? 0U : (i + 1U); }
    static constexpr size_type prev_idx(size_type i) noexcept { return (i == 0U) ? (kSlots - 1U) : (i - 1U); }

    size_type physical_index(size_type logical_index) const noexcept {
        ZTL_EXPECTS(logical_index < size());
        const size_type physical = m_tail + logical_index;
        return (physical >= kSlots) ? (physical - kSlots) : physical;
    }

    pointer       storage_ptr(size_type index) noexcept { return m_storage.ptr(index); }
    const_pointer storage_ptr(size_type index) const noexcept { return m_storage.ptr(index); }

    reference       storage_ref(size_type index) noexcept { return m_storage.ref(index); }
    const_reference storage_ref(size_type index) const noexcept { return m_storage.ref(index); }

    template <typename... Args>
    void construct_at(size_type index, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
        m_storage.construct(index, std::forward<Args>(args)...);
    }

    void destroy_at(size_type index) noexcept { m_storage.destroy(index); }

    class construction_guard final {
    public:
        explicit construction_guard(ring_buffer_base& owner) noexcept : m_owner{owner}, m_active{true} {}

        construction_guard(const construction_guard&)            = delete;
        construction_guard& operator=(const construction_guard&) = delete;

        ~construction_guard() noexcept {
            if (m_active) {
                m_owner.clear();
            }
        }

        void release() noexcept { m_active = false; }

    private:
        ring_buffer_base& m_owner;
        bool              m_active;
    };

    void copy_construct_from(const ring_buffer_base& rhs) {
        static_assert(std::is_copy_constructible<T>::value,
                      "ring_buffer_base copy construction: T must be copy-constructible");
        ZTL_EXPECTS(empty());

        construction_guard guard{*this};
        const size_type    count = rhs.size();
        for (size_type i = 0U; i < count; ++i) {
            construct_at(m_head, rhs[i]);
            m_head = next_idx(m_head);
        }
        guard.release();
    }

    void move_construct_from(ring_buffer_base& rhs) noexcept(std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_move_constructible<T>::value,
                      "ring_buffer_base move construction: T must be move-constructible");
        ZTL_EXPECTS(empty());

        construction_guard guard{*this};
        const size_type    count = rhs.size();
        for (size_type i = 0U; i < count; ++i) {
            construct_at(m_head, std::move(rhs[i]));
            m_head = next_idx(m_head);
        }
        guard.release();
    }

    struct push_context {
        size_type head;
        size_type old_tail;
        size_type new_tail;
        bool      overflow;
    };

    /// @brief Decide whether a push is accepted and record the required state transition.
    ///
    /// This deliberately does not mutate @c m_tail and does not destroy the
    /// overwritten element. The new value is constructed first and the old
    /// element is destroyed in commit_push(). This preserves self-referential
    /// pushes such as `buffer.push_back(buffer.front())` when the buffer is full.
    template <typename OverflowPolicy>
    bool prepare_push(push_context& context) noexcept {
        static_assert(
            details::has_overflow_policy<OverflowPolicy>::value,
            "ring_buffer_base: OverflowPolicy must provide static bool on_overflow(size_t&, size_t) noexcept");

        context.head     = m_head;
        context.old_tail = m_tail;
        context.new_tail = m_tail;
        context.overflow = full();

        if (!context.overflow) {
            return true;
        }

        size_type candidate_tail = m_tail;
        if (!OverflowPolicy::on_overflow(candidate_tail, kSlots)) {
            return false;
        }

        if (candidate_tail != next_idx(context.old_tail)) {
            return false;
        }

        context.new_tail = candidate_tail;
        return true;
    }

    /// @brief Publish a successfully constructed back element.
    ///
    /// @pre The new element has already been constructed at context.head.
    void commit_push(const push_context& context) noexcept {
        if (context.overflow) {
            destroy_at(context.old_tail);
            m_tail = context.new_tail;
        }

        m_head = next_idx(context.head);
    }

    /// @note The popper moves (or copies) the value out of the slot. The slot
    ///       is then destroyed immediately. T's destructor must tolerate
    ///       being called on a moved-from object (true for all standard types,
    ///       but user-defined types should verify this).
    template <typename Popper, typename Result>
    typename std::enable_if<std::is_void<Result>::value, Result>::type pop_front_impl(Popper& popper) {
        ZTL_EXPECTS(!empty());
        const size_type old_tail = m_tail;
        popper(storage_ref(old_tail));
        destroy_at(old_tail);
        m_tail = next_idx(m_tail);
    }

    template <typename Popper, typename Result>
    typename std::enable_if<!std::is_void<Result>::value, Result>::type pop_front_impl(Popper& popper) {
        ZTL_EXPECTS(!empty());
        const size_type old_tail = m_tail;
        Result          result   = popper(storage_ref(old_tail));
        destroy_at(old_tail);
        m_tail = next_idx(m_tail);
        return result;
    }

    template <typename Popper, typename Result>
    typename std::enable_if<std::is_void<Result>::value, Result>::type pop_back_impl(Popper& popper) {
        ZTL_EXPECTS(!empty());
        const size_type old_head = prev_idx(m_head);
        popper(storage_ref(old_head));
        destroy_at(old_head);
        m_head = old_head;
    }

    template <typename Popper, typename Result>
    typename std::enable_if<!std::is_void<Result>::value, Result>::type pop_back_impl(Popper& popper) {
        ZTL_EXPECTS(!empty());
        const size_type old_head = prev_idx(m_head);
        Result          result   = popper(storage_ref(old_head));
        destroy_at(old_head);
        m_head = old_head;
        return result;
    }

    size_type        m_head;
    size_type        m_tail;
    raw_storage_type m_storage;
};

template <typename T, std::size_t C>
void swap(ring_buffer_base<T, C>& a, ring_buffer_base<T, C>& b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
}

// ════════════════════════════════════════════════════════════════════════
//  ring_buffer
// ════════════════════════════════════════════════════════════════════════

/// @brief Fixed-capacity owning ring buffer with stored popper/overflow policies.
///
/// This wrapper preserves the previous four-template-parameter API while the
/// storage and non-policy algorithms live in @c ring_buffer_base<T, Capacity>.
///
/// @tparam T               Element type. Does not need to be default-constructible.
/// @tparam Capacity        Maximum number of elements. Must be >= 1.
/// @tparam PopperT         Pop strategy. Default moves the popped value out.
/// @tparam OverflowPolicyT Push overflow policy. Default overwrites oldest.
///                         Must provide `static bool on_overflow(size_t&, size_t) noexcept`.
///                         See @ref OverwritePolicy for the full contract.
template <typename T, std::size_t Capacity, typename PopperT = default_popper<T>,
          typename OverflowPolicyT = OverwritePolicy>
class ring_buffer final {
    static_assert(details::has_overflow_policy<OverflowPolicyT>::value,
                  "ring_buffer: OverflowPolicyT must provide static bool on_overflow(size_t&, size_t) noexcept");

public:
    using type            = ring_buffer;
    using base_type       = ring_buffer_base<T, Capacity>;
    using value_type      = typename base_type::value_type;
    using size_type       = typename base_type::size_type;
    using difference_type = typename base_type::difference_type;
    using reference       = typename base_type::reference;
    using const_reference = typename base_type::const_reference;
    using pointer         = typename base_type::pointer;
    using const_pointer   = typename base_type::const_pointer;
    using popper_type     = PopperT;
    using overflow_policy = OverflowPolicyT;

    template <bool IsConst>
    using iterator_impl = typename base_type::template iterator_impl<IsConst>;

    using iterator       = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;

    ring_buffer() noexcept(std::is_nothrow_default_constructible<PopperT>::value) : m_base{}, m_popper{} {}

    explicit ring_buffer(PopperT p) noexcept(std::is_nothrow_move_constructible<PopperT>::value)
        : m_base{}, m_popper{std::move(p)} {}

    ring_buffer(const ring_buffer& rhs) : m_base{rhs.m_base}, m_popper{rhs.m_popper} {
        static_assert(std::is_copy_constructible<PopperT>::value,
                      "ring_buffer copy construction: PopperT must be copy-constructible");
    }

    ring_buffer& operator=(const ring_buffer& rhs) {
        static_assert(std::is_copy_assignable<PopperT>::value,
                      "ring_buffer::operator=(const ring_buffer&): PopperT must be copy-assignable");

        if (this != &rhs) {
            m_popper = rhs.m_popper;
            m_base   = rhs.m_base;
        }
        return *this;
    }

    ring_buffer(ring_buffer&& rhs) noexcept(std::is_nothrow_move_constructible<PopperT>::value &&
                                            std::is_nothrow_move_constructible<T>::value)
        : m_base{}, m_popper{std::move(rhs.m_popper)} {
        m_base = std::move(rhs.m_base);
    }

    ring_buffer& operator=(ring_buffer&& rhs) noexcept(std::is_nothrow_move_assignable<PopperT>::value &&
                                                       std::is_nothrow_move_constructible<T>::value) {
        static_assert(std::is_move_assignable<PopperT>::value,
                      "ring_buffer::operator=(ring_buffer&&): PopperT must be move-assignable");

        if (this != &rhs) {
            m_popper = std::move(rhs.m_popper);
            m_base   = std::move(rhs.m_base);
        }
        return *this;
    }

    ~ring_buffer() noexcept = default;

    bool                       empty() const noexcept { return m_base.empty(); }
    bool                       full() const noexcept { return m_base.full(); }
    size_type                  size() const noexcept { return m_base.size(); }
    static constexpr size_type capacity() noexcept { return base_type::capacity(); }

    reference       front() noexcept { return m_base.front(); }
    const_reference front() const noexcept { return m_base.front(); }

    reference       back() noexcept { return m_base.back(); }
    const_reference back() const noexcept { return m_base.back(); }

    reference       operator[](size_type index) noexcept { return m_base[index]; }
    const_reference operator[](size_type index) const noexcept { return m_base[index]; }

    /// @brief Push a copy at the back. Overflow is handled by OverflowPolicyT.
    void push_back(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        m_base.template push_back<OverflowPolicyT>(value);
    }

    /// @brief Push a moved value at the back. Overflow is handled by OverflowPolicyT.
    void push_back(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        m_base.template push_back<OverflowPolicyT>(std::move(value));
    }

    /// @brief Construct an element directly in the back slot.
    template <typename... Args>
    void emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
        m_base.template emplace_back<OverflowPolicyT>(std::forward<Args>(args)...);
    }

    /// @brief Push a copy only when space is available. Never overwrites.
    bool try_push_back(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        return m_base.try_push_back(value);
    }

    /// @brief Push a moved value only when space is available. Never overwrites.
    bool try_push_back(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        return m_base.try_push_back(std::move(value));
    }

    /// @brief Construct an element directly in the back slot only when space is available.
    template <typename... Args>
    bool try_emplace_back(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
        return m_base.try_emplace_back(std::forward<Args>(args)...);
    }

    /// @brief Remove the oldest element through PopperT, then destroy the slot.
    /// @pre !empty()
    auto pop_front() -> decltype(std::declval<base_type&>().pop_front(std::declval<PopperT&>())) {
        return m_base.pop_front(m_popper);
    }

    /// @brief Remove the newest element through PopperT, then destroy the slot.
    /// @pre !empty()
    auto pop_back() -> decltype(std::declval<base_type&>().pop_back(std::declval<PopperT&>())) {
        return m_base.pop_back(m_popper);
    }

    /// @brief Destroy all live elements and reset the indices to zero.
    void clear() noexcept { m_base.clear(); }

    /// @brief Swap contents and poppers. O(N), requires nothrow move construction of T.
    void swap(ring_buffer& rhs) noexcept(std::is_nothrow_move_constructible<T>::value &&
                                         noexcept(std::swap(std::declval<PopperT&>(), std::declval<PopperT&>()))) {
        if (this == &rhs) {
            return;
        }

        using std::swap;
        swap(m_base, rhs.m_base);
        swap(m_popper, rhs.m_popper);
    }

    iterator       begin() noexcept { return m_base.begin(); }
    iterator       end() noexcept { return m_base.end(); }
    const_iterator begin() const noexcept { return m_base.begin(); }
    const_iterator end() const noexcept { return m_base.end(); }
    const_iterator cbegin() const noexcept { return m_base.cbegin(); }
    const_iterator cend() const noexcept { return m_base.cend(); }

private:
    base_type m_base;
    PopperT   m_popper;
};

template <typename T, std::size_t C, typename P, typename O>
void swap(ring_buffer<T, C, P, O>& a, ring_buffer<T, C, P, O>& b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
}

} // namespace ztl

#endif // ZTL_RINGBUFFER_HPP
