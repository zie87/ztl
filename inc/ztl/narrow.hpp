#ifndef ZTL_NARROW_HPP
#define ZTL_NARROW_HPP

#include <utility>

namespace ztl
{

template<class T, class U>
constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

template<class T>
constexpr typename std::underlying_type<T>::type to_underlying(T e) noexcept
{
    using type = typename std::underlying_type<T>::type;
    return static_cast<type>(e);
}

} // namespace ztl

#endif // ZTL_NARROW_HPP
