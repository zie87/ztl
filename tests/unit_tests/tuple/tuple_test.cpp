#include "ztl/tuple/tuple.hpp"
#include "ztl/meta/type_trait/is_same.hpp"
#include "ztl/meta/type_trait/remove_cv.hpp"

#include "test_framework.hpp"

namespace {

struct empty_test_class_1 {};
struct empty_test_class_2 {};

}  // namespace

SUITE(tuple) {
    TEST(tuple_size) {
        {
            using empty_tuple = ztl::tuple<>;
            UT_CHECK_EQUAL(empty_tuple::size(), 0U);
            UT_CHECK_EQUAL(empty_tuple::size(), ztl::tuple_size_v<empty_tuple>);
        }
        {
            using tuple_type = ztl::tuple<int, short>;
            UT_CHECK_EQUAL(tuple_type::size(), 2U);
            UT_CHECK_EQUAL(tuple_type::size(), ztl::tuple_size_v<tuple_type>);
        }
        {
            using tuple_type = ztl::tuple<char>;
            UT_CHECK_EQUAL(tuple_type::size(), 1U);
            UT_CHECK_EQUAL(tuple_type::size(), ztl::tuple_size_v<tuple_type>);
        }
        {
            using tuple_type = ztl::tuple<char, int, char, char>;
            UT_CHECK_EQUAL(tuple_type::size(), 4U);
            UT_CHECK_EQUAL(tuple_type::size(), ztl::tuple_size_v<tuple_type>);
        }
    }
    TEST(tuple_get) {
        {
            using tuple_type = ztl::tuple<signed, unsigned, char>;
            const auto val   = ztl::tuple{1, 42U, 'a'};
            UT_CHECK_TRUE((ztl::meta::is_same_v<ztl::meta::remove_cv_t<decltype(val)>, tuple_type>));

            UT_CHECK_EQUAL(ztl::get<0>(val), 1);
            UT_CHECK_EQUAL(ztl::get<1>(val), 42U);
            UT_CHECK_EQUAL(ztl::get<2>(val), 'a');
        }
        {
            using tuple_type = ztl::tuple<signed, char, unsigned>;
            tuple_type val{};

            ztl::get<0>(val) = 17;
            ztl::get<1>(val) = 'f';
            ztl::get<2>(val) = 123U;

            UT_CHECK_EQUAL(ztl::get<0>(val), 17);
            UT_CHECK_EQUAL(ztl::get<1>(val), 'f');
            UT_CHECK_EQUAL(ztl::get<2>(val), 123U);
        }
        {
            UT_CHECK_EQUAL(ztl::get<0>(ztl::tuple{'a', 17}), 'a');
            UT_CHECK_EQUAL(ztl::get<1>(ztl::tuple{'a', 17}), 17);
        }
    }
}
