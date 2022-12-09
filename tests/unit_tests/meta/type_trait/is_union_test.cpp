#include "ztl/meta/type_trait/is_union.hpp"

#include "test_framework.hpp"

namespace {
struct A {};

union B {
    int a;
    float b;
};

struct C {
    B d;
};

}  // namespace

TEST(meta_type_trait_is_union) {
    UT_CHECK_FALSE((ztl::meta::is_union_v<A>));
    UT_CHECK_TRUE((ztl::meta::is_union_v<B>));
    UT_CHECK_FALSE((ztl::meta::is_union_v<C>));
    UT_CHECK_FALSE((ztl::meta::is_union_v<int>));
}
