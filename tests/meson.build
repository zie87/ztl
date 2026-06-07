doctest_dep = dependency('doctest', fallback: ['doctest', 'doctest_dep'])

ztl_test_exe = executable(
    'ztl-test',
    files(
        'doctest_main.cpp',
        'ztl/assert_tests.cpp',
        'ztl/byte_tests.cpp',
        'ztl/cobs_tests.cpp',
        'ztl/crc_tests.cpp',
        'ztl/endian_tests.cpp',
        'ztl/functional/function_ref_tests.cpp',
        'ztl/hash_tests.cpp',
        'ztl/iterator/iterator_concepts_tests.cpp',
        'ztl/iterator/iterator_interface_tests.cpp',
        'ztl/narrow_tests.cpp',
        'ztl/ring_buffer_tests.cpp',
        'ztl/serialize_tests.cpp',
        'ztl/span_tests.cpp',
        'ztl/units_tests.cpp',
        'ztl/utility_tests.cpp',
    ),
    dependencies: [ztl_dep, doctest_dep],
)

test('ztl-tests', ztl_test_exe, suite: ['ztl'])
