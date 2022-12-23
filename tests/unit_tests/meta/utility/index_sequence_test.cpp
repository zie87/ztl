#include "ztl/meta/utility/index_sequence.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

TEST(meta_utility_index_sequence) {
    using seq_type_1 = ztl::meta::make_index_sequence<5>;
    using seq_type_2 = ztl::meta::index_sequence<0, 1, 2, 3, 4>;

    UT_CHECK_TRUE((ztl::meta::is_same_v<seq_type_1, seq_type_2>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<typename seq_type_1::value_type, ztl::size_t>));
    UT_CHECK_EQUAL(seq_type_1::size(), 5U);
}
