#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

namespace {
struct test_tag {};
}  // namespace

TEST(meta_type_trait_is_same) {
    UT_CHECK_TRUE((ztl::meta::is_same_v<char, char>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<test_tag, test_tag>));

    UT_CHECK_FALSE((ztl::meta::is_same_v<int, short>));
    UT_CHECK_FALSE((ztl::meta::is_same_v<int, int&>));
    UT_CHECK_FALSE((ztl::meta::is_same_v<int, const int>));
}
