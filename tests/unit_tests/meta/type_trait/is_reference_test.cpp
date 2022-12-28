#include "ztl/meta/type_trait/is_reference.hpp"

#include "test_framework.hpp"

SUITE(meta_type_trait_is_reference) {
    TEST(is_lvalue_reference) {
        UT_CHECK_FALSE((ztl::meta::is_lvalue_reference_v<short>));
        UT_CHECK_TRUE((ztl::meta::is_lvalue_reference_v<short&>));
        UT_CHECK_FALSE((ztl::meta::is_lvalue_reference_v<short&&>));
    }
    TEST(is_rvalue_reference) {
        UT_CHECK_FALSE((ztl::meta::is_rvalue_reference_v<short>));
        UT_CHECK_FALSE((ztl::meta::is_rvalue_reference_v<short&>));
        UT_CHECK_TRUE((ztl::meta::is_rvalue_reference_v<short&&>));
    }
    TEST(is_reference) {
        UT_CHECK_FALSE((ztl::meta::is_reference_v<short>));
        UT_CHECK_TRUE((ztl::meta::is_reference_v<short&>));
        UT_CHECK_TRUE((ztl::meta::is_reference_v<short&&>));
    }
}
