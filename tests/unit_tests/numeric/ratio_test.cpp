#include "ztl/numeric/ratio.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

SUITE(numeric_ratio) {
    TEST(ratio_basics) {
        {
            using ratio_type = ztl::ratio<10>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ratio_type>));
            UT_CHECK_EQUAL((ratio_type::num), 10);
            UT_CHECK_EQUAL((ratio_type::den), 1);
        }
        {
            using ratio_type = ztl::ratio<1, 12>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ratio_type>));
            UT_CHECK_EQUAL((ratio_type::num), 1);
            UT_CHECK_EQUAL((ratio_type::den), 12);
        }
        {
            using ratio_type = ztl::ratio<-3, -2>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ztl::ratio<3, 2>>));
            UT_CHECK_EQUAL((ratio_type::num), 3);
            UT_CHECK_EQUAL((ratio_type::den), 2);
        }
        {
            using ratio_type = ztl::ratio<5, 10>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ztl::ratio<1, 2>>));
            UT_CHECK_EQUAL((ratio_type::num), 1);
            UT_CHECK_EQUAL((ratio_type::den), 2);
        }
        {
            using ratio_type = ztl::ratio<24, 6>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ztl::ratio<4, 1>>));
            UT_CHECK_EQUAL((ratio_type::num), 4);
            UT_CHECK_EQUAL((ratio_type::den), 1);
        }
        {
            using ratio_type = ztl::ratio<-3, 6>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ztl::ratio<-1, 2>>));
            UT_CHECK_EQUAL((ratio_type::num), -1);
            UT_CHECK_EQUAL((ratio_type::den), 2);
        }
        {
            using ratio_type = ztl::ratio<-15, 5>;

            UT_CHECK_TRUE((ztl::meta::is_same_v<typename ratio_type::type, ztl::ratio<-3, 1>>));
            UT_CHECK_EQUAL((ratio_type::num), -3);
            UT_CHECK_EQUAL((ratio_type::den), 1);
        }
    }

    TEST(ratio_equal) {
        {
            using lhs_type = ztl::ratio<3, 6>;
            using rhs_type = ztl::ratio<1, 2>;
            UT_CHECK_TRUE((ztl::ratio_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_not_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<3, 6>;
            using rhs_type = ztl::ratio<-1, 2>;
            UT_CHECK_FALSE((ztl::ratio_equal_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_not_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<3, 4>;
            using rhs_type = ztl::ratio<1, 2>;
            UT_CHECK_FALSE((ztl::ratio_equal_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_not_equal_v<lhs_type, rhs_type>));
        }
    }

    TEST(ratio_comparison) {
        {
            using lhs_type = ztl::ratio<3, 6>;
            using rhs_type = ztl::ratio<1, 2>;
            UT_CHECK_FALSE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<-3, 6>;
            using rhs_type = ztl::ratio<-1, 2>;
            UT_CHECK_FALSE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<-3, 6>;
            using rhs_type = ztl::ratio<1, 2>;
            UT_CHECK_TRUE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<3, 6>;
            using rhs_type = ztl::ratio<-1, 2>;
            UT_CHECK_FALSE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<1, 3>;
            using rhs_type = ztl::ratio<1, 2>;
            UT_CHECK_TRUE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<-1, 2>;
            using rhs_type = ztl::ratio<-3, 4>;
            UT_CHECK_TRUE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
        {
            using lhs_type = ztl::ratio<7, 8>;
            using rhs_type = ztl::ratio<2, 3>;
            UT_CHECK_FALSE((ztl::ratio_less_v<lhs_type, rhs_type>));
            UT_CHECK_FALSE((ztl::ratio_less_equal_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_v<lhs_type, rhs_type>));
            UT_CHECK_TRUE((ztl::ratio_greater_equal_v<lhs_type, rhs_type>));
        }
    }

    TEST(ratio_multiply) {
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<1, 3>;

            using ret_type = ztl::ratio_multiply<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 1);
            UT_CHECK_EQUAL((ret_type::den), 6);
        }
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<-3, 4>;

            using ret_type = ztl::ratio_multiply<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), -3);
            UT_CHECK_EQUAL((ret_type::den), 8);
        }
        {
            using lhs_type = ztl::ratio<-1, 6>;
            using rhs_type = ztl::ratio<-3, 10>;

            using ret_type = ztl::ratio_multiply<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 1);
            UT_CHECK_EQUAL((ret_type::den), 20);
        }
    }
    TEST(ratio_divide) {
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<2, 1>;

            using ret_type = ztl::ratio_divide<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 1);
            UT_CHECK_EQUAL((ret_type::den), 4);
        }
        {
            using lhs_type = ztl::ratio<2, 3>;
            using rhs_type = ztl::ratio<1, 6>;

            using ret_type = ztl::ratio_divide<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 4);
            UT_CHECK_EQUAL((ret_type::den), 1);
        }
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<-1, 5>;

            using ret_type = ztl::ratio_divide<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), -5);
            UT_CHECK_EQUAL((ret_type::den), 2);
        }
        {
            using lhs_type = ztl::ratio<-4, 3>;
            using rhs_type = ztl::ratio<-7, 8>;

            using ret_type = ztl::ratio_divide<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 32);
            UT_CHECK_EQUAL((ret_type::den), 21);
        }
    }
    TEST(ratio_add) {
        {
            using lhs_type = ztl::ratio<1, 5>;
            using rhs_type = ztl::ratio<3, 5>;

            using ret_type = ztl::ratio_add<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 4);
            UT_CHECK_EQUAL((ret_type::den), 5);
        }
        {
            using lhs_type = ztl::ratio<-4, 5>;
            using rhs_type = ztl::ratio<3, 5>;

            using ret_type = ztl::ratio_add<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), -1);
            UT_CHECK_EQUAL((ret_type::den), 5);
        }
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<1, 4>;

            using ret_type = ztl::ratio_add<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 3);
            UT_CHECK_EQUAL((ret_type::den), 4);
        }
    }
    TEST(ratio_subtract) {
        {
            using lhs_type = ztl::ratio<4, 5>;
            using rhs_type = ztl::ratio<3, 5>;

            using ret_type = ztl::ratio_subtract<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), 1);
            UT_CHECK_EQUAL((ret_type::den), 5);
        }
        {
            using lhs_type = ztl::ratio<1, 2>;
            using rhs_type = ztl::ratio<3, 4>;

            using ret_type = ztl::ratio_subtract<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), -1);
            UT_CHECK_EQUAL((ret_type::den), 4);
        }
        {
            using lhs_type = ztl::ratio<-1, 6>;
            using rhs_type = ztl::ratio<2, 3>;

            using ret_type = ztl::ratio_subtract<lhs_type, rhs_type>;

            UT_CHECK_EQUAL((ret_type::num), -5);
            UT_CHECK_EQUAL((ret_type::den), 6);
        }
    }
}
