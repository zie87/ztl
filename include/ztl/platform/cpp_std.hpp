#ifndef ZTL_PLATFORM_CPP_STD_HPP
#define ZTL_PLATFORM_CPP_STD_HPP

#define ZTL_CPLUSPLUS __cplusplus

#define ZTL_CPP98_OR_GREATER (ZTL_CPLUSPLUS >= 199711L)
#define ZTL_CPP11_OR_GREATER (ZTL_CPLUSPLUS >= 201103L)
#define ZTL_CPP14_OR_GREATER (ZTL_CPLUSPLUS >= 201402L)
#define ZTL_CPP17_OR_GREATER (ZTL_CPLUSPLUS >= 201703L)
#define ZTL_CPP20_OR_GREATER (ZTL_CPLUSPLUS >= 202002L)

#if ZTL_CPP11_OR_GREATER
#define ZTL_NOEXCEPT noexcept
#define ZTL_CONSTEXPR_11 constexpr
#else
#define ZTL_NOEXCEPT
#define ZTL_CONSTEXPR_11
#endif

#if ZTL_CPP14_OR_GREATER
#define ZTL_CONSTEXPR_14 constexpr
#else
#define ZTL_CONSTEXPR_14
#endif

#if ZTL_CPP17_OR_GREATER
#else
#endif

#if ZTL_CPP20_OR_GREATER
#define ZTL_CONSTEVAL consteval
#else
#define ZTL_CONSTEVAL
#endif

#endif  // ZTL_PLATFORM_CPP_STD_HPP