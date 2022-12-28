#include "ztl/meta/type_trait/remove_reference.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

TEST(meta_type_trait_remove_reference) {
    UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_reference_t<int>, int>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_reference_t<const int>, const int>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_reference_t<int&>, int>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_reference_t<int&&>, int>));
}
