#include "ztl/meta/type_trait/is_integral.hpp"

#include "test_framework.hpp"

namespace {
struct test_tag {};
}  // namespace

TEST(meta_type_trait_is_integral) {
    UT_CHECK_TRUE((ztl::meta::is_integral_v<bool>));
    
    UT_CHECK_TRUE((ztl::meta::is_integral_v<char>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<char8_t>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<char16_t>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<char32_t>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<wchar_t>));

    UT_CHECK_TRUE((ztl::meta::is_integral_v<short>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<int>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<long>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<long long>));

    UT_CHECK_TRUE((ztl::meta::is_integral_v<unsigned short>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<unsigned int>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<unsigned long>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<unsigned long long>));

    UT_CHECK_TRUE((ztl::meta::is_integral_v<const int>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<const unsigned long>));

    UT_CHECK_TRUE((ztl::meta::is_integral_v<volatile short>));
    UT_CHECK_TRUE((ztl::meta::is_integral_v<const volatile unsigned char>));

    UT_CHECK_FALSE((ztl::meta::is_integral_v<char*>));
    UT_CHECK_FALSE((ztl::meta::is_integral_v<int&>));
    UT_CHECK_FALSE((ztl::meta::is_integral_v<test_tag>));
}
