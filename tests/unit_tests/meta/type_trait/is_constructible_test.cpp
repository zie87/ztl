#include "ztl/meta/type_trait/is_constructible.hpp"

#include "test_framework.hpp"

namespace {
struct test_tag {
    test_tag();
    test_tag(int);
    test_tag(char, short);
};

struct test_tag_noexcept {
    test_tag_noexcept() noexcept;
    test_tag_noexcept(int) noexcept;
    test_tag_noexcept(char, short) noexcept;
};
}  // namespace

SUITE(meta_type_trait_is_constructible) {
    TEST(is_constructible) {
        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag_noexcept>));
        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag_noexcept, int>));
        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag_noexcept, char, short>));

        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag>));
        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag, int>));
        UT_CHECK_TRUE((ztl::meta::is_constructible_v<test_tag, char, short>));
    }
    TEST(is_nothrow_constructible) {
        UT_CHECK_TRUE((ztl::meta::is_nothrow_constructible_v<test_tag_noexcept>));
        UT_CHECK_TRUE((ztl::meta::is_nothrow_constructible_v<test_tag_noexcept, int>));
        UT_CHECK_TRUE((ztl::meta::is_nothrow_constructible_v<test_tag_noexcept, char, short>));

        UT_CHECK_FALSE((ztl::meta::is_nothrow_constructible_v<test_tag>));
        UT_CHECK_FALSE((ztl::meta::is_nothrow_constructible_v<test_tag, int>));
        UT_CHECK_FALSE((ztl::meta::is_nothrow_constructible_v<test_tag, char, short>));
    }
}
