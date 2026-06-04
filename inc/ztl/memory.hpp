#ifndef ZTL_MEMORY_HPP
#define ZTL_MEMORY_HPP

#include <memory>
#include <type_traits>

namespace ztl
{

namespace details
{

// clang-format off
template<int N>
struct priority_tag : priority_tag<N - 1> {};

template<>
struct priority_tag<0> {};
// clang-format on

template<class T>
constexpr T* to_address_impl(T* p, priority_tag<2>) noexcept
{
    static_assert(!std::is_function<T>::value, "to_address() does not accept function pointers");
    return p;
}

template<class Ptr>
constexpr auto to_address_impl(const Ptr& p, priority_tag<1>) noexcept
    -> decltype(std::pointer_traits<Ptr>::to_address(p))
{
    return std::pointer_traits<Ptr>::to_address(p);
}

template<class Ptr>
constexpr auto to_address_impl(const Ptr& p, priority_tag<0>) noexcept
    -> decltype(to_address_impl(p.operator->(), priority_tag<2>{}))
{
    return to_address_impl(p.operator->(), priority_tag<2>{});
}

} // namespace details

template<class T>
constexpr T* to_address(T* p) noexcept
{
    return details::to_address_impl(p, details::priority_tag<2>{});
}

template<class Ptr>
constexpr auto to_address(const Ptr& p) noexcept
    -> decltype(details::to_address_impl(p, details::priority_tag<2>{}))
{
    return details::to_address_impl(p, details::priority_tag<2>{});
}

} // namespace ztl

#endif // ZTL_MEMORY_HPP
