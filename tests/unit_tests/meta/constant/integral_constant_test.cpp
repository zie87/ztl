#include "ztl/meta/constant/integral_constant.hpp"

#include "test_framework.hpp"

TEST(meta_constants_integral_constants) {
    {
        using constant = ztl::meta::integral_constant<std::size_t, 12>;

        constant obj;
        constexpr std::size_t value = obj;
        UT_CHECK_EQUAL(value, 12U);
        UT_CHECK_EQUAL(obj(), 12U);
    }

    {
        using constant = ztl::meta::integral_constant<int, -17>;

        constant obj;
        constexpr int value = obj;
        UT_CHECK_EQUAL(value, -17);
        UT_CHECK_EQUAL(obj(), -17);
    }

    {
        using constant = ztl::meta::integral_constant<bool, true>;

        constant obj;
        UT_CHECK_TRUE(obj);
        UT_CHECK_TRUE(obj());
    }
}
