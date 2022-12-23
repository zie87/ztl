#include "ztl/tuple/detail/is_tuple.hpp"

#include "test_framework.hpp"

namespace {
struct empty_class {};
}  // namespace

TEST(tuple_detail_is_tuple) {
    UT_CHECK_FALSE((ztl::detail::is_tuple_v<int>));
    UT_CHECK_FALSE((ztl::detail::is_tuple_v<empty_class>));

    UT_CHECK_TRUE((ztl::detail::is_tuple_v<ztl::tuple<int> >));
    UT_CHECK_TRUE((ztl::detail::is_tuple_v<ztl::tuple<empty_class> >));
    UT_CHECK_TRUE((ztl::detail::is_tuple_v<ztl::tuple<empty_class, int, empty_class> >));
}
