#include "ztl/utility/endian/endian_swap.hpp"
#include "ztl/types.hpp"

#include "test_framework.hpp"

TEST(utility_endian_endian_swap) {
    {
        const ztl::uint8_t val = 0b00001111;
        
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::little>(val)), val);

        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::little>(val)), val);
    }

    {
        const ztl::uint16_t val = 0xCAFE;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::little>(val)), val);

        const ztl::uint16_t swapped = 0xFECA;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::big>(val)), swapped);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::little>(val)), swapped);
    }

    {
        const ztl::uint32_t val = 0xDEAD'BEEF;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::little>(val)), val);

        constexpr ztl::uint32_t swapped = 0xEFBE'ADDE;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::big>(val)), swapped);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::little>(val)), swapped);
    }

    {
        const ztl::uint64_t val = 0x0123'4567'89AB'CDEF;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::little>(val)), val);

        constexpr ztl::uint64_t swapped = 0xEFCD'AB89'6745'2301;
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::little, ztl::endian::big>(val)), swapped);
        UT_CHECK_EQUAL((ztl::endian_swap<ztl::endian::big, ztl::endian::little>(val)), swapped);
    }

    if constexpr (ztl::endian::native == ztl::endian::little)
    {
        const ztl::uint32_t val = 0xDEAD'BEEF;
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::native>(val)), val);
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::little>(val)), val);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::native>(val)), val);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::little>(val)), val);

        constexpr ztl::uint32_t swapped = 0xEFBE'ADDE;
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::big>(val)), swapped);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::big>(swapped)), val);
    } else {
        const ztl::uint32_t val = 0xDEAD'BEEF;
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::native>(val)), val);
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::big>(val)), val);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::native>(val)), val);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::big>(val)), val);

        constexpr ztl::uint32_t swapped = 0xEFBE'ADDE;
        UT_CHECK_EQUAL((ztl::to_endian<ztl::endian::little>(val)), swapped);
        UT_CHECK_EQUAL((ztl::from_endian<ztl::endian::little>(swapped)), val);
    }
}
