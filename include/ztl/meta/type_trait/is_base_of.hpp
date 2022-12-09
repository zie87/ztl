#ifndef ZTL_META_TYPE_TRAIT_IS_BASE_OF_HPP
#define ZTL_META_TYPE_TRAIT_IS_BASE_OF_HPP

#include "ztl/meta/constant/bool_constant.hpp"
#include "ztl/meta/type_trait/is_class.hpp"

#include <type_traits>

// based on: https://en.cppreference.com/w/cpp/types/is_base_of
namespace ztl::meta {

namespace detail {
template <typename BASE_T>
auto test_is_ptr_convertible(const volatile BASE_T*) -> true_type;

template <typename>
auto test_is_ptr_convertible(const volatile void*) -> false_type;

template <typename, typename>
auto test_is_base_of(...) -> true_type;

template <typename BASE_T, typename DERIVED_T>
auto test_is_base_of(int) -> decltype(test_is_ptr_convertible<BASE_T>(static_cast<DERIVED_T*>(nullptr)));
}  // namespace detail

template <typename BASE_T, typename DERIVED_T>
struct is_base_of
    : bool_constant<is_class_v<BASE_T> &&
                    is_class_v<DERIVED_T>&& decltype(detail::test_is_base_of<BASE_T, DERIVED_T>(0))::value> {};

template <typename BASE_T, typename DERIVED_T>
inline constexpr bool is_base_of_v = is_base_of<BASE_T, DERIVED_T>::value;

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_IS_BASE_OF_HPP
