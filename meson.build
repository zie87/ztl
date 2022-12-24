project(
    'ztl',
    'cpp',
    version: '0.1',
    license: 'BSL-1.0',
    default_options: ['cpp_std=c++20', 'warning_level=3'],
)

inc_dir = include_directories('include')
ztl_dep = declare_dependency(include_directories: inc_dir)

ztl_little_endian = 0x0002
ztl_big_endian = 0x0004

add_project_arguments(
    '-DZTL_CONFIG_LITTLE_ENDIAN=' + ztl_little_endian.to_string(),
    language: 'cpp',
)
add_project_arguments(
    '-DZTL_CONFIG_BIG_ENDIAN=' + ztl_big_endian.to_string(),
    language: 'cpp',
)

if build_machine.endian() == 'little'
    add_project_arguments(
        '-DZTL_CONFIG_NATIVE_ENDIAN=' + ztl_little_endian.to_string(),
        language: 'cpp',
    )
else
    add_project_arguments(
        '-DZTL_CONFIG_NATIVE_ENDIAN=' + ztl_big_endian.to_string(),
        language: 'cpp',
    )
endif

subdir('tests')
