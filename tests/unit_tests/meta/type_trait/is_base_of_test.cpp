#include "ztl/meta/type_trait/is_base_of.hpp"

#include "test_framework.hpp"

namespace {

class A {};
class B : private A {};
class C : private B {};
class D {};
class E : public D {};

}  // namespace

TEST(meta_type_trait_is_base_of) {
    UT_CHECK_TRUE((ztl::meta::is_base_of_v<A, A>));
    UT_CHECK_TRUE((ztl::meta::is_base_of_v<A, B>));
    UT_CHECK_TRUE((ztl::meta::is_base_of_v<A, C>));
    UT_CHECK_TRUE((ztl::meta::is_base_of_v<A, const B>));

    UT_CHECK_FALSE((ztl::meta::is_base_of_v<A, D>));
    UT_CHECK_FALSE((ztl::meta::is_base_of_v<B, A>));
    UT_CHECK_FALSE((ztl::meta::is_base_of_v<A&, B&>));
    UT_CHECK_FALSE((ztl::meta::is_base_of_v<int, int>));

    UT_CHECK_TRUE((ztl::meta::is_base_of_v<D, E>));
}
