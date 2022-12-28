#include "ztl/utility/forward.hpp"

#include "test_framework.hpp"

namespace {

struct forward_test {
    forward_test(int&&) noexcept : rval(true) {}
    forward_test(int&) noexcept : lval(true) {}

    bool rval = false;
    bool lval = false;
};

template <typename T, typename U>
auto make_test_obj(U&& arg) -> T {
    return T(ztl::forward<U>(arg));
}

}  // namespace

TEST(utility_forward) {
    {
        auto ret = make_test_obj<forward_test>(2);
        UT_CHECK_TRUE(ret.rval);
        UT_CHECK_FALSE(ret.lval);
    }
    {
        int val  = 3;
        auto ret = make_test_obj<forward_test>(val);
        UT_CHECK_FALSE(ret.rval);
        UT_CHECK_TRUE(ret.lval);
    }
}
