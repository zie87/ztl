project(
    'ztl',
    'cpp',
    version: '0.1',
    default_options: ['cpp_std=c++20', 'warning_level=3'],
)

inc_dir = include_directories('include')
ztl_dep = declare_dependency(include_directories: inc_dir)

subdir('tests')
