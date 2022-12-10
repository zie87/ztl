#include "ztl/utility/endian/byteswap.hpp"
#include "ztl/types.hpp"

#include "test_framework.hpp"

TEST(utility_endian_byteswap) {
    {
        const ztl::uint8_t val = 0b00001111;
        UT_CHECK_EQUAL(ztl::byteswap(val), val);
    }

    {
        const ztl::uint16_t val = 0xCAFE;
        UT_CHECK_EQUAL(ztl::byteswap(val), 0xFECA);
    }

    {
        const ztl::uint32_t val = 0xDEAD'BEEF;
        UT_CHECK_EQUAL(ztl::byteswap(val), 0xEFBE'ADDE);
    }

    {
        const ztl::uint64_t val = 0x0123'4567'89AB'CDEF;
        UT_CHECK_EQUAL(ztl::byteswap(val), 0xEFCD'AB89'6745'2301);
    }

    {
        constexpr std::int32_t val1     = 0xDEAD'CAFE;
        constexpr std::int32_t val2     = ztl::byteswap(val1);
        constexpr std::int32_t expected = 0xFECA'ADDE;
        static_assert((val2 == expected));
    }
}
