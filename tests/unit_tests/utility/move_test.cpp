#include "ztl/utility/move.hpp"

#include "test_framework.hpp"

namespace {

struct move_test {
    move_test() noexcept = default;
    move_test(move_test&& other) noexcept : moved_in(true) { other.moved_out = true; }
    move_test(const move_test&) noexcept {}

    bool moved_in  = false;
    bool moved_out = false;
};

}  // namespace

TEST(utility_move) {
    move_test default_obj{};
    move_test moved_obj(ztl::move(default_obj));

    UT_CHECK_TRUE(moved_obj.moved_in);
    UT_CHECK_TRUE(default_obj.moved_out);
}
