#include "ztl/meta/sequence/push_front.hpp"
#include "ztl/meta/sequence/empty.hpp"
#include "ztl/meta/sequence/size.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

TEST(meta_sequence_push_front) {
    {
        using seq_type = ztl::meta::sequence<>;
        UT_CHECK_TRUE((ztl::meta::empty_v<seq_type>));

        using test_type = ztl::meta::push_front_t<seq_type, char>;
        UT_CHECK_TRUE((ztl::meta::is_same_v<test_type, ztl::meta::sequence<char>>));
        UT_CHECK_EQUAL((ztl::meta::size_v<test_type>), 1U);
        UT_CHECK_FALSE((ztl::meta::empty_v<test_type>));
    }
    {
        using seq_type  = ztl::meta::sequence<char>;
        using test_type = ztl::meta::push_front_t<seq_type, short>;
        UT_CHECK_TRUE((ztl::meta::is_same_v<test_type, ztl::meta::sequence<short, char>>));
        UT_CHECK_EQUAL((ztl::meta::size_v<test_type>), 2U);
    }
    {
        using seq_type  = ztl::meta::sequence<char, float, int>;
        using test_type = ztl::meta::push_front_t<seq_type, char>;
        UT_CHECK_TRUE((ztl::meta::is_same_v<test_type, ztl::meta::sequence<char, char, float, int>>));
        UT_CHECK_EQUAL((ztl::meta::size_v<test_type>), 4U);
    }
}
