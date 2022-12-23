#include "ztl/meta/utility/integer_sequence.hpp"
#include "ztl/meta/type_trait/is_same.hpp"

#include "test_framework.hpp"

TEST(meta_utility_integer_sequence) {
    using seq_type_1 = ztl::meta::make_integer_sequence<short, 4>;
    using seq_type_2 = ztl::meta::integer_sequence<short, 0, 1, 2, 3>;

    UT_CHECK_TRUE((ztl::meta::is_same_v<seq_type_1, seq_type_2>));
    UT_CHECK_TRUE((ztl::meta::is_same_v<typename seq_type_1::value_type, short>));
    UT_CHECK_EQUAL(seq_type_1::size(), 4U);
}
