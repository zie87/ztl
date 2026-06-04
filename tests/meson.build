doctest_dep = dependency('doctest', fallback: ['doctest', 'doctest_dep'])

ztl_test_exe = executable(
    'ztl-test',
    files('doctest_main.cpp', 'ztl/crc_tests.cpp', 'ztl/byte_tests.cpp', 'ztl/endian_tests.cpp'),
    dependencies: [ztl_dep, doctest_dep],
)

test('ztl-tests', ztl_test_exe, suite: ['ztl'])
