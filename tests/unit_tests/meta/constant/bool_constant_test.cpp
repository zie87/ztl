#include "ztl/meta/constant/bool_constant.hpp"

#include "test_framework.hpp"

TEST(meta_constants_bool_constants) {
    {
        ztl::meta::true_type obj;
        UT_CHECK_TRUE(obj);
        UT_CHECK_TRUE(obj());
    }

    {
        ztl::meta::false_type obj;
        UT_CHECK_FALSE(obj);
        UT_CHECK_FALSE(obj());
    }
}
