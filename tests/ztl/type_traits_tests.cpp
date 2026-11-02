#include "ztl/type_traits.hpp"

#include <doctest/doctest.h>

#include <type_traits>

namespace {
// instantiating ::value fails to compile, so it must never be reached
template <typename T>
struct must_not_instantiate {
    static_assert(sizeof(T) == 0, "conjunction must short-circuit");
    static constexpr bool value = true;
};
} // namespace

TEST_CASE("conjunction is true only when every trait is true") {
    static_assert(ztl::conjunction<>::value, "empty conjunction is true");
    static_assert(ztl::conjunction<std::true_type>::value, "single true");
    static_assert(!ztl::conjunction<std::false_type>::value, "single false");
    static_assert(ztl::conjunction<std::true_type, std::true_type, std::true_type>::value, "all true");
    static_assert(!ztl::conjunction<std::true_type, std::false_type, std::true_type>::value, "one false");
    CHECK(ztl::conjunction<std::true_type, std::is_integral<int>>::value);
}

TEST_CASE("conjunction stops at the first false trait") {
    static_assert(!ztl::conjunction<std::false_type, must_not_instantiate<int>>::value,
                  "traits after the first false one must not be instantiated");
    // like std::conjunction, it derives from the first false trait
    static_assert(std::is_base_of<std::is_integral<float>,
                                  ztl::conjunction<std::true_type, std::is_integral<float>, std::false_type>>::value,
                  "conjunction derives from the first false trait");
    CHECK_FALSE(ztl::conjunction<std::is_integral<float>, must_not_instantiate<int>>::value);
}
