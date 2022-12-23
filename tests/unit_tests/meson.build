unit_tests_srcs = [
    'meta/constant/bool_constant_test.cpp',
    'meta/constant/integral_constant_test.cpp',
    'meta/sequence/push_back_test.cpp',
    'meta/sequence/push_front_test.cpp',
    'meta/type_trait/is_base_of_test.cpp',
    'meta/type_trait/is_class_test.cpp',
    'meta/type_trait/is_integral_test.cpp',
    'meta/type_trait/is_same_test.cpp',
    'meta/type_trait/is_union_test.cpp',
    'meta/type_trait/remove_cv_test.cpp',
    'meta/utility/index_sequence_test.cpp',
    'meta/utility/integer_sequence_test.cpp',
    'numeric/ratio_test.cpp',
    'tuple/is_tuple_test.cpp',
    'tuple/tuple_test.cpp',
    'type/byte_test.cpp',
    'utility/endian/byteswap_test.cpp',
    'utility/endian/endian_swap_test.cpp',
    'utility/source_location_test.cpp',
    'main.cpp',
]

subproject('unittest-cpp', default_options: 'warning_level=0')
unittest_dep = dependency('unittest-cpp')
unit_tests_inc_dir = include_directories('.')
unit_tests_exec = executable(
    'ztl_unit_tests',
    unit_tests_srcs,
    include_directories: unit_tests_inc_dir,
    dependencies: [unittest_dep, ztl_dep],
    override_options: ['cpp_std=c++20', 'warning_level=3'],
)

test('ztl-unit-tests', unit_tests_exec, suite: 'ztl')
