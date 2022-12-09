# ztl::meta:::is_base_of

``` C++
namespace ztl::meta {

template <typename BASE_T, typename DERIVED_T>
struct is_base_of;

template <typename BASE_T, typename DERIVED_T>
inline constexpr bool is_base_of_v = is_base_of<BASE_T, DERIVED_T>::value;

} // namespace ztl::meta
```
This traits indicates if `DERIVED_T` is derived from `BASE_T` or are the same non-union class. This detects a derived class also in the case, where `BASE_T` 
is a `protected` or `private` base class.

##  Possible Implementation

### boost::is_base_of

Pre `C++11` boost::type_traits had already a kind of implementation for `is_base_of`: [boost::is_base_and_derived](https://github.com/boostorg/type_traits/blob/boost-1.31.0/include/boost/type_traits/is_base_and_derived.hpp). This implementation differs from the current version: a check on the same class will be false in the case of `boost::is_base_and_derived`, in the standard version this will be true.

``` C++
// boost::is_base_and_derived (simplified)

typedef char (&yes)[1];
typedef char (&no)[2];

template <typename BASE_T, typename DERIVED_T>
struct Host
{
  operator BASE_T*() const;
  operator DERIVED_T*();
};

template <typename BASE_T, typename DERIVED_T>
struct is_base_and_derived
{
  template <typename T> 
  static yes check(DERIVED_T*, T);
  static no check(BASE_T*, int);

  static const bool value = sizeof(check(Host<BASE_T,DERIVED_T>(), int())) == sizeof(yes);
};

//Test sample
class Base {};
class Derived : private Base {};

//Expression is true.
int test[is_base_of<Base,Derived>::value && !is_base_of<Derived,Base>::value];
```

This implementation uses the old `sizeof`-trick to detect the result of an expression at compile time. The return values of the `check`-function calls will differ in there size (`sizeof(yes)` vs `sizeof(no)`). With evaluating of this size the result can verified on compile time with out evaluating the function it self.

This implemntation uses the best match evaluation to find out if a ``DERIVED_T


* [How does `is_base_of` work?](https://stackoverflow.com/questions/2910979/how-does-is-base-of-work)

``` C++ 
// std::is_base_of

template <typename Base> std::true_type is_base_of_test_func( Base* );
template <typename Base> std::false_type is_base_of_test_func( void* );
template <typename Base, typename Derived>
using pre_is_base_of = decltype( is_base_of_test_func<Base>( std::declval<Derived*>() ) );

template <typename Base, typename Derived, typename = void>
struct pre_is_base_of2 : public std::true_type {};

template<typename ...> using void_t = void;
template <typename Base, typename Derived>
struct pre_is_base_of2<Base, Derived, void_t<pre_is_base_of<Base, Derived>>> : public pre_is_base_of<Base, Derived>{};


template <typename Base, typename Derived>
struct is_base_of : public std::conditional_t<std::is_class<Base>::value && std::is_class<Derived>::value,
                                              pre_is_base_of2<Base, Derived>,
                                              std::false_type>
{
};
```

* [Explanation of possible implementation of `std::is_base_of`](https://stackoverflow.com/questions/43632614/explanation-of-possible-implementation-of-stdis-base-of)
* [wiki discussion](https://en.cppreference.com/w/Talk:cpp/types/is_base_of)
