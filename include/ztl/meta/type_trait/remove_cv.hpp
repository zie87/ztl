#ifndef ZTL_META_TYPE_TRAIT_REMOVE_CV_HPP
#define ZTL_META_TYPE_TRAIT_REMOVE_CV_HPP

namespace ztl::meta {

// clang-format off
template <typename T> struct remove_const          { using type = T; };
template <typename T> struct remove_const<const T> { using type = T; };
template <typename T> using remove_const_t = typename remove_const<T>::type;
// clang-format on

// clang-format off
template <typename T> struct remove_volatile             { using type = T; };
template <typename T> struct remove_volatile<volatile T> { using type = T; };
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;
// clang-format on

// clang-format off
template <typename T> struct remove_cv { using type = remove_volatile_t<remove_const_t<T>>; };
template <typename T> using remove_cv_t = typename remove_cv<T>::type;
// clang-format on

}  // namespace ztl::meta

#endif  // ZTL_META_TYPE_TRAIT_REMOVE_CV_HPP
