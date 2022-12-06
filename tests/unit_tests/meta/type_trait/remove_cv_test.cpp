#include "ztl/meta/type_trait/remove_cv.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

SUITE(meta_type_trait_remove_cv) {
    TEST(remove_const) {
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<char>, char>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<const char>, char>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<volatile char>, volatile char>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<const volatile char>, volatile char>));

        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<const char*>, const char*>));
    }

    TEST(remove_volatile) {
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_volatile_t<int>, int>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_volatile_t<const int>, const int>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_volatile_t<volatile int>, int>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_volatile_t<const volatile int>, const int>));

        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_const_t<volatile int*>, volatile int*>));
    }

    TEST(remove_cv) {
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<short>, short>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<const short>, short>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<volatile short>, short>));
        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<const volatile short>, short>));

        UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<const volatile short*>, const volatile short*>));
    }
}
