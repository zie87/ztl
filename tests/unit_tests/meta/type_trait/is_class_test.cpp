#include "ztl/meta/type_trait/is_class.hpp"

#include "test_framework.hpp"

namespace {

struct A {};
class B {};
enum class E {};
union U {
    class UC {};
};
}  // namespace

TEST(meta_type_trait_is_class) {
    UT_CHECK_TRUE((ztl::meta::is_class_v<A>));
    UT_CHECK_TRUE((ztl::meta::is_class_v<B>));
    UT_CHECK_FALSE((ztl::meta::is_class_v<B*>));
    UT_CHECK_FALSE((ztl::meta::is_class_v<B&>));
    UT_CHECK_TRUE((ztl::meta::is_class_v<const B>));
    UT_CHECK_FALSE((ztl::meta::is_class_v<E>));
    UT_CHECK_FALSE((ztl::meta::is_class_v<int>));

    UT_CHECK_FALSE((ztl::meta::is_class_v<U>));
    UT_CHECK_TRUE((ztl::meta::is_class_v<U::UC>));

    // incomplete types
    UT_CHECK_TRUE((ztl::meta::is_class_v<struct S>));
    UT_CHECK_TRUE((ztl::meta::is_class_v<class C>));
}
