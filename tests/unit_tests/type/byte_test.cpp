#include "ztl/type/byte.hpp"

#include "test_framework.hpp"

TEST(type_byte) {
    {
        ztl::byte b{42};
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b00101010);

        b <<= 2;
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b10101000);

        b >>= 2;
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b00101010);
    }

    {
        const ztl::byte b1{42};
        UT_CHECK_EQUAL(ztl::to_integer<int>(b1 << 1), 0b01010100);
        UT_CHECK_EQUAL(ztl::to_integer<int>(b1 >> 1), 0b00010101);
    }

    {
        ztl::byte b{0b00101010};
        b |= ztl::byte{0b11110000};
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b11111010);

        b &= ztl::byte{0b11110000};
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b11110000);

        b ^= ztl::byte{0b11111111};
        UT_CHECK_EQUAL(ztl::to_integer<int>(b), 0b00001111);

        UT_CHECK_EQUAL(ztl::to_integer<int>(~b), 0b11110000);
    }
}
