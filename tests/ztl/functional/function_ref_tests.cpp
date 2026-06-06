#include <ztl/functional/function_ref.hpp>

#include <doctest/doctest.h>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace {

struct data {
    int value;
};

int free_function_no_args() noexcept {
    return 123;
}
char free_function_one_arg(char value) {
    return value;
}
short free_function_short_result() {
    return 38;
}
int free_pointer_function(data* ptr) noexcept {
    return ptr->value;
}

template <typename T>
T generic_free_function() noexcept {
    return T(38);
}

struct static_function {
    static int sum(int a, int b) noexcept { return a + b; }
};

struct functor {
    void operator()(bool new_value) noexcept { value = new_value; }
    int  operator()() const noexcept { return 17; }

    bool value;
};

struct member_test_class {
    bool set_and_report(int value) noexcept {
        last_value = value;
        return true;
    }

    char const_member(bool value) const {
        static_cast<void>(value);
        return '7';
    }

    int last_value;
};

struct lvalue_only_callable {
    void operator()() {}
};

struct noncopyable_callable {
    explicit noncopyable_callable(int& counter) noexcept : counter_ptr(&counter) {}

    noncopyable_callable(const noncopyable_callable&)            = delete;
    noncopyable_callable& operator=(const noncopyable_callable&) = delete;

    int operator()(int value) noexcept {
        ++(*counter_ptr);
        return value + *counter_ptr;
    }

    int* counter_ptr;
};

struct incompatible_callable {
    int operator()(const char*) noexcept { return 0; }
};

using int_no_args_function_pointer  = int (*)();
using int_data_arg_function_pointer = int (*)(data*);

int_no_args_function_pointer get_free_function_no_args_pointer() noexcept {
    return &free_function_no_args;
}

int_data_arg_function_pointer get_free_pointer_function_pointer() noexcept {
    return &free_pointer_function;
}

void disturb_stack() noexcept {
    volatile int scratch[32];
    for (std::size_t i = 0U; i < 32U; ++i) {
        scratch[i] = static_cast<int>(i);
    }

    const int keep_scratch_used = scratch[31];
    static_cast<void>(keep_scratch_used);
}

struct stored_no_args_callback {
    explicit stored_no_args_callback(ztl::function_ref<int()> callback) noexcept : action(callback) {}

    ztl::function_ref<int()> action;
};

struct stored_data_callback {
    explicit stored_data_callback(ztl::function_ref<int(data*)> callback) noexcept : action(callback) {}

    ztl::function_ref<int(data*)> action;
};

template <typename...>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

template <typename T, typename = void>
struct is_bool_testable : std::false_type {};

template <typename T>
struct is_bool_testable<T, void_t<decltype(static_cast<bool>(std::declval<T const&>()))>> : std::true_type {};

template <typename T, typename = void>
struct has_nullptr_equal : std::false_type {};

template <typename T>
struct has_nullptr_equal<T, void_t<decltype(std::declval<T const&>() == nullptr)>> : std::true_type {};

template <typename T, typename = void>
struct has_nullptr_not_equal : std::false_type {};

template <typename T>
struct has_nullptr_not_equal<T, void_t<decltype(std::declval<T const&>() != nullptr)>> : std::true_type {};

template <typename T, typename = void>
struct has_target_type : std::false_type {};

template <typename T>
struct has_target_type<T, void_t<decltype(std::declval<T const&>().target_type())>> : std::true_type {};

template <typename T, typename = void>
struct has_target : std::false_type {};

template <typename T>
struct has_target<T, void_t<decltype(std::declval<T&>().template target<int (*)()>())>> : std::true_type {};

} // namespace

TEST_CASE("function_ref has non-owning value semantics") {
    static_assert(!std::is_default_constructible<ztl::function_ref<void()>>::value,
                  "function_ref must always reference a callable");
    static_assert(std::is_copy_constructible<ztl::function_ref<void()>>::value,
                  "function_ref should be copy constructible");
    static_assert(std::is_copy_assignable<ztl::function_ref<void()>>::value, "function_ref should be copy assignable");
    static_assert(std::is_trivially_copyable<ztl::function_ref<void()>>::value,
                  "P0792R14 specifies every function_ref specialization as trivially copyable");
    static_assert(std::is_nothrow_copy_constructible<ztl::function_ref<void()>>::value,
                  "copy construction should be noexcept");
    static_assert(std::is_nothrow_copy_assignable<ztl::function_ref<void()>>::value,
                  "copy assignment should be noexcept");

    static_assert(std::is_constructible<ztl::function_ref<void()>, lvalue_only_callable&>::value,
                  "callable lvalues may be referenced");
    static_assert(!std::is_constructible<ztl::function_ref<void()>, lvalue_only_callable>::value,
                  "temporary callable objects must not be accepted");
    static_assert(!std::is_assignable<ztl::function_ref<void()>&, lvalue_only_callable&>::value,
                  "rebinding from arbitrary callable assignment should stay deleted");
    static_assert(std::is_assignable<ztl::function_ref<int()>&, int_no_args_function_pointer>::value,
                  "rebinding through function pointers should remain possible");
    static_assert(std::is_nothrow_constructible<ztl::function_ref<int()>, int_no_args_function_pointer>::value,
                  "construction from a function pointer should be noexcept");
    static_assert(std::is_nothrow_constructible<ztl::function_ref<void(bool)>, functor&>::value,
                  "construction from a callable reference should be noexcept");
}

TEST_CASE("function_ref omits std::function-style nullable and target APIs") {
    using ref_type = ztl::function_ref<int()>;

    static_assert(!std::is_constructible<ref_type, std::nullptr_t>::value, "function_ref has no null state");
    static_assert(!is_bool_testable<ref_type>::value, "function_ref should not provide operator bool");
    static_assert(!has_nullptr_equal<ref_type>::value, "function_ref should not compare equal to nullptr");
    static_assert(!has_nullptr_not_equal<ref_type>::value, "function_ref should not compare unequal to nullptr");
    static_assert(!has_target_type<ref_type>::value, "function_ref should not expose target_type()");
    static_assert(!has_target<ref_type>::value, "function_ref should not expose target<T>()");
}

TEST_CASE("function_ref rejects non-callable and incompatible targets") {
    static_assert(!std::is_constructible<ztl::function_ref<int(int)>, data&>::value,
                  "non-callable object references should not bind");
    static_assert(!std::is_constructible<ztl::function_ref<int(int)>, incompatible_callable&>::value,
                  "wrong call signatures should not bind");
}

TEST_CASE("function_ref rejects direct pointer-to-member construction") {
    using member_function_pointer       = bool (member_test_class::*)(int);
    using const_member_function_pointer = char (member_test_class::*)(bool) const;
    using member_object_pointer         = int data::*;

    static_assert(
        !std::is_constructible<ztl::function_ref<bool(member_test_class&, int)>, member_function_pointer>::value,
        "P0792R14 Behavior B.2 rejects direct pointer-to-member functions");
    static_assert(!std::is_constructible<ztl::function_ref<char(member_test_class const&, bool)>,
                                         const_member_function_pointer>::value,
                  "P0792R14 Behavior B.2 rejects direct const pointer-to-member functions");
    static_assert(!std::is_constructible<ztl::function_ref<int&(data&)>, member_object_pointer>::value,
                  "P0792R14 Behavior B.2 rejects direct pointer-to-member objects");
}

TEST_CASE("function_ref binds lambdas") {
    SUBCASE("capture-less lambda prvalue converts to a function pointer") {
        ztl::function_ref<int()> func_ref([]() { return 42; });
        CHECK(func_ref() == 42);
    }

    SUBCASE("capturing lambda lvalue is referenced") {
        char observed = 'a';
        auto lambda   = [&observed](char value) noexcept { observed = value; };

        ztl::function_ref<void(char)> func_ref(lambda);
        func_ref('g');

        CHECK(observed == 'g');
    }
}

TEST_CASE("function_ref binds function objects") {
    SUBCASE("mutable functor lvalue") {
        functor                       object = {false};
        ztl::function_ref<void(bool)> func_ref(object);
        func_ref(true);

        CHECK(object.value == true);
    }

    SUBCASE("const call operator on mutable functor") {
        functor                  object = {false};
        ztl::function_ref<int()> func_ref(object);

        CHECK(func_ref() == 17);
    }

    SUBCASE("const functor lvalue") {
        const functor            object = {false};
        ztl::function_ref<int()> func_ref(object);

        CHECK(func_ref() == 17);
    }

    SUBCASE("non-copyable callable lvalue") {
        int                  calls = 0;
        noncopyable_callable object(calls);

        ztl::function_ref<int(int)> func_ref(object);

        CHECK(func_ref(10) == 11);
        CHECK(func_ref(10) == 12);
        CHECK(calls == 2);
    }

    SUBCASE("operator() is const-qualified but still calls mutable targets") {
        functor                             object = {false};
        const ztl::function_ref<void(bool)> func_ref(object);

        func_ref(true);

        CHECK(object.value == true);
    }
}

TEST_CASE("function_ref binds free and static functions") {
    SUBCASE("free function without arguments") {
        ztl::function_ref<int()> func_ref(free_function_no_args);
        CHECK(func_ref() == 123);
    }

    SUBCASE("free function with argument") {
        ztl::function_ref<char(char)> func_ref(free_function_one_arg);
        CHECK(func_ref('A') == 'A');
    }

    SUBCASE("static member function") {
        ztl::function_ref<int(int, int)> func_ref(static_function::sum);
        CHECK(func_ref(7, 11) == 18);
    }

    SUBCASE("generic free function instantiation") {
        ztl::function_ref<short()> func_ref(generic_free_function<short>);
        CHECK(func_ref() == 38);
    }

    SUBCASE("return type may be converted") {
        ztl::function_ref<long()> func_ref(free_function_short_result);
        CHECK(func_ref() == 38L);
    }

    SUBCASE("function pointer returned by value remains callable after storage") {
        stored_no_args_callback options(get_free_function_no_args_pointer());

        disturb_stack();

        CHECK(options.action() == 123);
    }

    SUBCASE("function and function pointer initialization have the same observable behavior") {
        data object = {182};

        stored_data_callback from_function(free_pointer_function);
        stored_data_callback from_pointer(&free_pointer_function);
        stored_data_callback from_returned_pointer(get_free_pointer_function_pointer());

        disturb_stack();

        CHECK(from_function.action(&object) == 182);
        CHECK(from_pointer.action(&object) == 182);
        CHECK(from_returned_pointer.action(&object) == 182);
    }

    SUBCASE("function pointer assignment rebinds through implicit construction") {
        ztl::function_ref<int()> func_ref(free_function_no_args);

        func_ref = get_free_function_no_args_pointer();

        CHECK(func_ref() == 123);
    }

    SUBCASE("compile-time function address assignment rebinds through implicit construction") {
        ztl::function_ref<int()> func_ref(free_function_no_args);

        func_ref = ZTL_NONTYPE(&free_function_no_args);

        CHECK(func_ref() == 123);
    }
}

TEST_CASE("function_ref binds compile-time function addresses") {
    SUBCASE("free function address") {
        ztl::function_ref<int()> func_ref(ZTL_NONTYPE(&free_function_no_args));
        CHECK(func_ref() == 123);
    }

    SUBCASE("generic free function address") {
        ztl::function_ref<long()> func_ref(ZTL_NONTYPE(&generic_free_function<long>));
        CHECK(func_ref() == 38L);
    }
}

TEST_CASE("function_ref binds member functions through compile-time addresses") {
    SUBCASE("non-const member function on object reference") {
        member_test_class            object = {0};
        ztl::function_ref<bool(int)> func_ref(ZTL_NONTYPE(&member_test_class::set_and_report), object);

        CHECK(func_ref(5) == true);
        CHECK(object.last_value == 5);
    }

    SUBCASE("const member function on mutable object") {
        member_test_class             object = {0};
        ztl::function_ref<char(bool)> func_ref(ZTL_NONTYPE(&member_test_class::const_member), object);

        CHECK(func_ref(false) == '7');
    }

    SUBCASE("const member function on const object") {
        const member_test_class       object = {0};
        ztl::function_ref<char(bool)> func_ref(ZTL_NONTYPE(&member_test_class::const_member), object);

        CHECK(func_ref(false) == '7');
    }

    SUBCASE("member object pointer returns a reference") {
        data                      object = {182};
        ztl::function_ref<int&()> func_ref(ZTL_NONTYPE(&data::value), object);
        func_ref() = 200;

        CHECK(object.value == 200);
    }

    SUBCASE("member object pointer can bind through object pointer") {
        data                      object = {182};
        ztl::function_ref<int&()> func_ref(ZTL_NONTYPE(&data::value), &object);
        func_ref() = 201;

        CHECK(object.value == 201);
    }
}

TEST_CASE("function_ref binds one object parameter") {
    data                     object = {182};
    ztl::function_ref<int()> func_ref(ZTL_NONTYPE(&free_pointer_function), &object);

    CHECK(func_ref() == 182);
}

#if defined(ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE) && ZTL_FUNCTION_REF_HAS_NOEXCEPT_FUNCTION_TYPE
TEST_CASE("function_ref preserves noexcept signatures when the compiler supports them") {
    ztl::function_ref<int() noexcept> func_ref(free_function_no_args);

    static_assert(std::is_constructible<ztl::function_ref<int() noexcept>, decltype(&free_function_no_args)>::value,
                  "noexcept targets should bind to noexcept function_ref");
    static_assert(!std::is_constructible<ztl::function_ref<int() noexcept>, decltype(&free_function_one_arg)>::value,
                  "incompatible noexcept signatures should not bind");
    static_assert(noexcept(func_ref()), "noexcept function_ref should expose noexcept operator()");
    CHECK(func_ref() == 123);
}
#endif
