#include <doctest/doctest.h>

#include <ztl/hash.hpp>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace {
static_assert(ztl::fnv1a32_cstr("") == 0x811C9DC5u, "FNV-1a empty string mismatch");
static_assert(ztl::fnv1a32_cstr("a") == 0xE40C292Cu, "FNV-1a 'a' mismatch");
static_assert(ztl::fnv1a32_cstr("hello") == 0x4F9F2CABu, "FNV-1a 'hello' mismatch");
static_assert(ztl::fnv1a32_cstr("foobar") == 0xBF9CF968u, "FNV-1a 'foobar' mismatch");

static_assert(ztl::fnv1a32_literal("") == 0x811C9DC5u, "FNV-1a empty literal mismatch");
static_assert(ztl::fnv1a32_literal("a") == 0xE40C292Cu, "FNV-1a literal 'a' mismatch");
static_assert(ztl::fnv1a32_literal("hello") == 0x4F9F2CABu, "FNV-1a literal 'hello' mismatch");
static_assert(ztl::fnv1a32_literal("a\0b") == 0x10F3ABD2u, "FNV-1a embedded null literal mismatch");
static_assert(ztl::fnv1a32_literal("\0") == 0x050C5D1Fu, "FNV-1a single null byte literal mismatch");

static_assert(noexcept(ztl::fnv1a32_cstr("hello")), "fnv1a32_cstr must be noexcept");
static_assert(noexcept(ztl::fnv1a32_literal("hello")), "fnv1a32_literal must be noexcept");
static_assert(noexcept(ztl::fnv1a32_bytes(nullptr, 0U)), "fnv1a32_bytes must be noexcept");
} // namespace

TEST_CASE("fnv1a32_cstr matches known FNV-1a vectors") {
    CHECK(ztl::fnv1a32_cstr("") == 0x811C9DC5u);
    CHECK(ztl::fnv1a32_cstr("a") == 0xE40C292Cu);
    CHECK(ztl::fnv1a32_cstr("hello") == 0x4F9F2CABu);
    CHECK(ztl::fnv1a32_cstr("foobar") == 0xBF9CF968u);
}

TEST_CASE("fnv1a32_cstr stops at the first null terminator") {
    const char* const text = "a\0b";

    CHECK(ztl::fnv1a32_cstr(text) == 0xE40C292Cu);
    CHECK(ztl::fnv1a32_cstr(text) == ztl::fnv1a32_cstr("a"));
}

TEST_CASE("fnv1a32_literal hashes the literal extent excluding only the final terminator") {
    CHECK(ztl::fnv1a32_literal("") == 0x811C9DC5u);
    CHECK(ztl::fnv1a32_literal("hello") == 0x4F9F2CABu);
    CHECK(ztl::fnv1a32_literal("a\0b") == 0x10F3ABD2u);
    CHECK(ztl::fnv1a32_literal("\0") == 0x050C5D1Fu);
}

TEST_CASE("fnv1a32_bytes hashes arbitrary byte ranges") {
    const unsigned char data[] = {static_cast<unsigned char>(0x00U), static_cast<unsigned char>(0x01U),
                                  static_cast<unsigned char>(0x02U), static_cast<unsigned char>(0x03U),
                                  static_cast<unsigned char>(0xFEU), static_cast<unsigned char>(0xFFU)};

    CHECK(ztl::fnv1a32_bytes(data, sizeof(data)) == 0x0D396C06u);
    CHECK(ztl::fnv1a32_bytes(nullptr, 0U) == 0x811C9DC5u);
}

TEST_CASE("fnv1a32 seed parameter supports incremental hashing") {
    const std::uint32_t partial = ztl::fnv1a32_cstr("he");
    const std::uint32_t full    = ztl::fnv1a32_cstr("llo", partial);

    CHECK(full == 0x4F9F2CABu);
    CHECK(full == ztl::fnv1a32_cstr("hello"));
}
