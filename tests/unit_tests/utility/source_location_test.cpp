#include "ztl/utility/source_location.hpp"

#include "test_framework.hpp"

#include <cstring>

namespace {

auto test_function() noexcept -> ztl::source_location {
    const auto loc = ztl::source_location::current();
    return loc;
}

}  // namespace

TEST(utility_source_location) {
    const auto loc = test_function();

    UT_CHECK_EQUAL(loc.line(), 10U);
    UT_CHECK_EQUAL(std::strcmp(loc.function_name(), "test_function"), 0);
    UT_CHECK_EQUAL(std::strcmp(loc.file_name(), __FILE__), 0);
}
