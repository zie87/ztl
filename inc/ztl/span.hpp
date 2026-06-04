#ifndef ZTL_SPAN_HPP
#define ZTL_SPAN_HPP

#include "ztl/type_traits.hpp"
#include "ztl/memory.hpp"
#include "ztl/assert.hpp"
#include "ztl/narrow.hpp"
#include "ztl/byte.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>

namespace ztl
{

// NOTE: this is a gsl like span implementation and not fully compatible with a C++20 std::span (https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-views). The implemenation is based on the MS.GSL but adapted for personal needs (IAR 8.5, C++14).

constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

template<typename T, std::size_t EXTENT_V = dynamic_extent>
class span;

namespace details
{
template<typename T>
struct span_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = ztl::remove_cv_t<T>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    constexpr span_iterator() = default;

    constexpr span_iterator(pointer begin, pointer end, pointer current)
    : m_begin(begin), m_end(end), m_current(current)
    {
        ZTL_EXPECTS(begin != nullptr);
        ZTL_EXPECTS(end != nullptr);
        ZTL_EXPECTS(current != nullptr);
        ZTL_EXPECTS(m_begin <= m_current && current <= m_end);
    }

    constexpr reference operator*() const noexcept
    {
        ZTL_EXPECTS(m_current != m_end);
        return *m_current;
    }

    constexpr pointer operator->() const noexcept
    {
        ZTL_EXPECTS(m_current != m_end);
        return m_current;
    }
    constexpr span_iterator& operator++() noexcept
    {
        ZTL_EXPECTS(m_current != m_end);

        ++m_current;
        return *this;
    }

    constexpr span_iterator operator++(int) noexcept
    {
        span_iterator ret = *this;
        ++*this;
        return ret;
    }

    constexpr span_iterator& operator--() noexcept
    {
        ZTL_EXPECTS(m_begin != m_current);
        --m_current;
        return *this;
    }

    constexpr span_iterator operator--(int) noexcept
    {
        span_iterator ret = *this;
        --*this;
        return ret;
    }

    constexpr span_iterator& operator+=(const difference_type n) noexcept
    {
        ZTL_EXPECTS(m_begin != nullptr);
        ZTL_EXPECTS(m_end != nullptr);
        ZTL_EXPECTS(m_current != nullptr);

        if (n != 0)
        {
            ZTL_EXPECTS(m_begin && m_current && m_end);
        }
        if (n > 0)
        {
            ZTL_EXPECTS(m_end - m_current >= n);
        }
        if (n < 0)
        {
            ZTL_EXPECTS(m_current - m_begin >= -n);
        }
        m_current += n;
        return *this;
    }

    constexpr span_iterator operator+(const difference_type n) const noexcept
    {
        span_iterator ret = *this;
        ret += n;
        return ret;
    }

    friend constexpr span_iterator operator+(const difference_type n,
                                             const span_iterator&  rhs) noexcept
    {
        return rhs + n;
    }

    constexpr span_iterator& operator-=(const difference_type n) noexcept
    {
        ZTL_EXPECTS(m_begin && m_current && m_end);

        if (n > 0)
        {
            ZTL_EXPECTS(m_current - m_begin >= n);
        }
        if (n < 0)
        {
            ZTL_EXPECTS(m_end - m_current >= -n);
        }
        m_current -= n;
        return *this;
    }

    constexpr span_iterator operator-(const difference_type n) const noexcept
    {
        span_iterator ret = *this;
        ret -= n;
        return ret;
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr difference_type operator-(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        ZTL_EXPECTS(m_begin == rhs.m_begin && m_end == rhs.m_end);
        ZTL_EXPECTS(m_current != nullptr);
        ZTL_EXPECTS(rhs.m_current != nullptr);
        return m_current - rhs.m_current;
    }

    constexpr reference operator[](const difference_type n) const noexcept { return *(*this + n); }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator==(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        ZTL_EXPECTS(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current == rhs.m_current;
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator!=(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator<(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        ZTL_EXPECTS(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current < rhs.m_current;
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator>(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        return rhs < *this;
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator<=(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        return !(rhs < *this);
    }

    template<class OTHER_T,
             std::enable_if_t<std::is_same<std::remove_cv_t<OTHER_T>, value_type>::value, int> = 0>
    constexpr bool operator>=(const span_iterator<OTHER_T>& rhs) const noexcept
    {
        return !(*this < rhs);
    }

private:
    template<typename>
    friend struct span_iterator;

    pointer m_begin{nullptr};
    pointer m_end{nullptr};
    pointer m_current{nullptr};
};

template<std::size_t EXTENT_V>
struct extent_type
{
    using size_type = std::size_t;

    static constexpr size_type extent = EXTENT_V;

    constexpr extent_type() noexcept = default;

    constexpr explicit extent_type(extent_type<dynamic_extent> ext);

    constexpr explicit extent_type(size_type size) { ZTL_EXPECTS(size == extent); }

    constexpr size_type size() const noexcept { return extent; }
};

template<>
class extent_type<dynamic_extent>
{
public:
    using size_type = std::size_t;

    static constexpr size_type extent = dynamic_extent;
    constexpr extent_type() noexcept : m_size(0U) {}

    template<size_type EXTENT_V>
    constexpr explicit extent_type(extent_type<EXTENT_V> ext) : m_size(ext.size())
    {
    }

    constexpr explicit extent_type(size_type size) : m_size(size)
    {
        ZTL_EXPECTS(size != dynamic_extent);
    }

    constexpr size_type size() const noexcept { return m_size; }

private:
    size_type m_size;
};

template<std::size_t EXTENT_V>
constexpr extent_type<EXTENT_V>::extent_type(extent_type<dynamic_extent> ext)
{
    ZTL_EXPECTS(ext.size() == EXTENT_V);
}

template<std::size_t FROM_V, std::size_t TO_V>
struct is_extent_conversion_allowed : ztl::bool_constant<FROM_V == TO_V || TO_V == dynamic_extent>
{
};

template<typename FROM_T, typename TO_T>
struct is_element_type_conversion_allowed
: ztl::bool_constant<std::is_convertible<FROM_T (*)[], TO_T (*)[]>::value>
{
};

template<typename T, std::size_t EXTENT_V, std::size_t OFFSET_V, std::size_t N>
struct calc_subspan_type
{
    static constexpr std::size_t extent
        = N != dynamic_extent ? N : (EXTENT_V != dynamic_extent ? EXTENT_V - OFFSET_V : EXTENT_V);

    using type = span<T, extent>;
};

} // namespace details

template<typename T, std::size_t EXTENT_V>
class span
{
public:
    using element_type    = T;
    using value_type      = std::remove_cv_t<T>;
    using size_type       = std::size_t;
    using pointer         = element_type*;
    using const_pointer   = const element_type*;
    using reference       = element_type&;
    using const_reference = const element_type&;
    using difference_type = std::ptrdiff_t;

    using iterator         = details::span_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    static constexpr size_type extent = EXTENT_V;

    template<std::size_t EXT_V                                            = EXTENT_V,
             std::enable_if_t<EXT_V == dynamic_extent || EXT_V == 0, int> = 0>
    constexpr span() noexcept : m_storage(nullptr, details::extent_type<0>{})
    {
    }

    constexpr span(const span& other) noexcept            = default;
    constexpr span& operator=(const span& other) noexcept = default;

    template<std::size_t EXT_V = EXTENT_V, std::enable_if_t<EXT_V != dynamic_extent, int> = 0>
    constexpr explicit span(pointer ptr, size_type count) noexcept : m_storage(ptr, count)
    {
        ZTL_EXPECTS(count == EXTENT_V);
    }

    template<std::size_t EXT_V = EXTENT_V, std::enable_if_t<EXT_V == dynamic_extent, int> = 0>
    constexpr span(pointer ptr, size_type count) noexcept : m_storage(ptr, count)
    {
    }

    template<std::size_t EXT_V = EXTENT_V, std::enable_if_t<EXT_V != dynamic_extent, int> = 0>
    constexpr explicit span(pointer first, pointer last) noexcept
    : m_storage(first, checked_distance(first, last))
    {
        ZTL_EXPECTS(last - first == static_cast<difference_type>(EXTENT_V));
    }

    template<std::size_t EXT_V = EXTENT_V, std::enable_if_t<EXT_V == dynamic_extent, int> = 0>
    constexpr span(pointer first, pointer last) noexcept
    : m_storage(first, checked_distance(first, last))
    {
    }

    template<std::size_t EXT_V                                            = EXTENT_V,
             std::enable_if_t<EXT_V == dynamic_extent || EXT_V == 0, int> = 0>
    constexpr span(std::nullptr_t, size_type count) noexcept
    : span(static_cast<pointer>(nullptr), count)
    {
        ZTL_EXPECTS(count == 0U);
    }

    template<std::size_t N,
             std::enable_if_t<details::is_extent_conversion_allowed<N, EXTENT_V>::value, int> = 0>
    constexpr span(element_type (&arr)[N]) noexcept
    : m_storage(unchecked_pointer{arr}, details::extent_type<N>())
    {
    }

    template<typename OTHER_T,
             std::size_t N,
             std::enable_if_t<
                 (details::is_extent_conversion_allowed<N, EXTENT_V>::value
                  && details::is_element_type_conversion_allowed<OTHER_T, element_type>::value),
                 int>
             = 0>
    constexpr span(std::array<OTHER_T, N>& arr) noexcept
    : m_storage(unchecked_pointer{arr.data()}, details::extent_type<N>())
    {
    }

    template<typename OTHER_T,
             std::size_t N,
             std::enable_if_t<(details::is_extent_conversion_allowed<N, EXTENT_V>::value
                               && details::is_element_type_conversion_allowed<const OTHER_T,
                                                                              element_type>::value),
                              int>
             = 0>
    constexpr span(const std::array<OTHER_T, N>& arr) noexcept
    : m_storage(unchecked_pointer{arr.data()}, details::extent_type<N>())
    {
    }

    template<typename OTHER_T, std::size_t N>
    span(std::array<OTHER_T, N>&&) = delete;

    template<typename OTHER_T, std::size_t N>
    span(const std::array<OTHER_T, N>&&) = delete;

    template<class OTHER_T,
             std::size_t N,
             std::size_t EXT_V = EXTENT_V,
             std::enable_if_t<
                 (EXT_V == dynamic_extent || EXT_V == N)
                     && details::is_element_type_conversion_allowed<OTHER_T, element_type>::value,
                 int>
             = 0>
    constexpr span(const span<OTHER_T, N>& other) noexcept
    : m_storage(other.data(), details::extent_type<N>(other.size()))
    {
    }

    template<class OTHER_T,
             std::size_t N,
             std::size_t EXT_V = EXTENT_V,
             std::enable_if_t<
                 EXT_V != dynamic_extent && N == dynamic_extent
                     && details::is_element_type_conversion_allowed<OTHER_T, element_type>::value,
                 int>
             = 0>
    constexpr explicit span(const span<OTHER_T, N>& other) noexcept
    : m_storage(other.data(), details::extent_type<N>(other.size()))
    {
    }

    constexpr size_type size() const noexcept { return m_storage.size(); }
    constexpr size_type size_bytes() const noexcept
    {
        ZTL_EXPECTS(size() <= (std::numeric_limits<size_type>::max() / sizeof(element_type)));
        return size() * sizeof(element_type);
    }
    constexpr bool empty() const noexcept { return size() == 0; }

    constexpr pointer data() const noexcept { return m_storage.data(); }

    constexpr reference operator[](size_type idx) const noexcept { return data()[idx]; }
    constexpr reference at(size_type idx) const noexcept
    {
        ZTL_EXPECTS(idx < size());
        return data()[idx];
    }

    constexpr reference front() const noexcept
    {
        ZTL_EXPECTS(size() > 0);
        return data()[0];
    }

    constexpr reference back() const noexcept
    {
        ZTL_EXPECTS(size() > 0);
        return data()[size() - 1];
    }

    constexpr iterator begin() const noexcept
    {
        const auto start_data = data();
        const auto end_data   = add(start_data, size());
        return {start_data, end_data, start_data};
    }

    constexpr iterator end() const noexcept
    {
        const auto start_data = data();
        const auto end_data   = add(start_data, size());
        return {start_data, end_data, end_data};
    }

    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

    template<std::size_t OFFSET_V, std::size_t N = dynamic_extent>
    constexpr auto subspan() const noexcept ->
        typename details::calc_subspan_type<T, EXTENT_V, OFFSET_V, N>::type
    {
        static_assert(
            extent == dynamic_extent
                || (extent >= OFFSET_V && (N == dynamic_extent || N <= extent - OFFSET_V)),
            "subspan() cannot extract more elements from a span than it contains.");
        ZTL_EXPECTS((size() >= OFFSET_V) && (N == dynamic_extent || (N <= size() - OFFSET_V)));

        using type          = typename details::calc_subspan_type<T, extent, OFFSET_V, N>::type;
        const size_type ext = N == dynamic_extent ? size() - OFFSET_V : N;
        return type{add(data(), OFFSET_V), ext};
    }

    constexpr span<element_type, dynamic_extent>
    subspan(size_type offset, size_type count = dynamic_extent) const noexcept
    {
        return make_subspan(offset, count, subspan_selector<extent>{});
    }

    template<std::size_t N>
    constexpr span<element_type, N> first() const noexcept
    {
        static_assert(extent == dynamic_extent || N <= extent,
                      "first() cannot extract more elements from a span than it contains.");
        ZTL_EXPECTS(N <= size());
        return span<element_type, N>{data(), N};
    }

    constexpr span<element_type, dynamic_extent> first(size_type count) const noexcept
    {
        ZTL_EXPECTS(count <= size());
        return {data(), count};
    }

    template<std::size_t N>
    constexpr span<element_type, N> last() const noexcept
    {
        static_assert(extent == dynamic_extent || N <= extent,
                      "last() cannot extract more elements from a span than it contains.");
        ZTL_EXPECTS(N <= size());
        return span<element_type, N>{add(data(), size() - N), N};
    }

    constexpr span<element_type, dynamic_extent> last(size_type count) const noexcept
    {
        ZTL_EXPECTS(count <= size());
        return make_subspan(size() - count, dynamic_extent, subspan_selector<extent>{});
    }

private:
    struct unchecked_pointer
    {
        pointer p;
    };

    static inline constexpr pointer add(pointer p, size_type n) noexcept
    {
        ZTL_EXPECTS(p != nullptr || n == 0U);
        return n == 0U ? p : p + n;
    }

    static inline constexpr size_type checked_distance(pointer first, pointer last) noexcept
    {
        if (first == last)
        {
            return 0U;
        }

        ZTL_EXPECTS(first != nullptr);
        ZTL_EXPECTS(last != nullptr);

        const auto diff = last - first;
        ZTL_EXPECTS(diff >= 0);

        return narrow_cast<size_type>(diff);
    }
    // NOTE: private constructor to avoid not need checks for subspans
    constexpr span(unchecked_pointer ptr, size_type count) noexcept : m_storage(ptr, count) {}

    template<std::size_t N>
    class subspan_selector
    {
    };

    template<std::size_t N>
    constexpr span<element_type, dynamic_extent>
    make_subspan(size_type offset, size_type count, subspan_selector<N>) const noexcept
    {
        const span<element_type, dynamic_extent> tmp(*this);
        return tmp.subspan(offset, count);
    }

    constexpr span<element_type, dynamic_extent>
    make_subspan(size_type offset, size_type count, subspan_selector<dynamic_extent>) const noexcept
    {
        ZTL_EXPECTS(size() >= offset);

        if (count == dynamic_extent)
        {
            return {unchecked_pointer{add(data(), offset)}, size() - offset};
        }

        ZTL_EXPECTS(size() - offset >= count);
        return {unchecked_pointer{add(data(), offset)}, count};
    }

    template<class EXTENT_T>
    class storage_type : public EXTENT_T
    {
        using base = EXTENT_T;

    public:
        template<class OTHER_EXTENT_T>
        constexpr storage_type(unchecked_pointer data, OTHER_EXTENT_T ext)
        : base(ext), m_data(data.p)
        {
        }

        template<class OTHER_EXTENT_T>
        constexpr storage_type(pointer data, OTHER_EXTENT_T ext) : base(ext), m_data(data)
        {
            ZTL_EXPECTS(data || base::size() == 0);
        }

        constexpr pointer data() const noexcept { return m_data; }

    private:
        pointer m_data;
    };

    storage_type<details::extent_type<extent>> m_storage;
};

#if defined(__cpp_deduction_guides) && (__cpp_deduction_guides >= 201606)
// Deduction Guides
template<typename T, std::size_t N>
span(T (&)[N]) -> span<T, N>;

template<typename T, std::size_t N>
span(std::array<T, N>&) -> span<T, N>;

template<typename T, std::size_t N>
span(const std::array<T, N>&) -> span<const T, N>;
#endif

template<class T, std::size_t N>
inline constexpr span<T, N> make_span(T (&arr)[N]) noexcept
{
    return span<T, N>{arr};
}

template<class T, std::size_t N>
inline constexpr span<T, N> make_span(std::array<T, N>& arr) noexcept
{
    return span<T, N>{arr};
}

template<class T, std::size_t N>
inline constexpr span<const T, N> make_span(const std::array<T, N>& arr) noexcept
{
    return span<const T, N>{arr};
}

template<class T>
inline constexpr span<T, dynamic_extent> make_span(T* ptr, std::size_t count) noexcept
{
    return span<T, dynamic_extent>{ptr, count};
}

template<class T>
inline constexpr span<T, dynamic_extent> make_span(T* first, T* last) noexcept
{
    return span<T, dynamic_extent>{first, last};
}

namespace details
{

template<class T, std::size_t EXTENT_V>
struct calculate_byte_size : std::integral_constant<std::size_t, sizeof(T) * EXTENT_V>
{
    static_assert(EXTENT_V <= dynamic_extent / sizeof(T), "Size is too big.");
};

template<class T>
struct calculate_byte_size<T, dynamic_extent> : std::integral_constant<std::size_t, dynamic_extent>
{
};
} // namespace details

template<class T, std::size_t EXTENT_V>
inline span<const ztl::byte, details::calculate_byte_size<T, EXTENT_V>::value>
as_bytes(span<T, EXTENT_V> s) noexcept
{
    using type = span<const ztl::byte, details::calculate_byte_size<T, EXTENT_V>::value>;
    return type{reinterpret_cast<const ztl::byte*>(s.data()), s.size_bytes()};
}

template<class T, std::size_t EXTENT_V, std::enable_if_t<!std::is_const<T>::value, int> = 0>
inline span<ztl::byte, details::calculate_byte_size<T, EXTENT_V>::value>
as_writable_bytes(span<T, EXTENT_V> s) noexcept
{
    using type = span<ztl::byte, details::calculate_byte_size<T, EXTENT_V>::value>;
    return type{reinterpret_cast<ztl::byte*>(s.data()), s.size_bytes()};
}

} // namespace ztl

#endif // ZTL_SPAN_HPP
