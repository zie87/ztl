#include "ztl/utility/source_location.hpp"

#include <utest.h>

#include <iostream>

UTEST_STATE();

int main(int argc, const char* const argv[]) {
    const ztl::source_location loc = ZTL_CURRENT_SOURCE_LOCATION;

    std::cout << "file: " << loc.file_name() << " line: " << loc.line()
              << " func: " << loc.function_name() << std::endl;

    return utest_main(argc, argv);
}
