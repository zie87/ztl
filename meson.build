project(
    'ztl',
    'cpp',
    version: '0.1.0',
    license: 'BSL-1.0',
    license_files: 'LICENSE',
    meson_version: '>=1.1.0',
    default_options: [
        'cpp_std=c++14',
        'warning_level=3',
        'buildtype=debugoptimized',
    ],
)


inc_dirs = include_directories('inc')

ztl_dep = declare_dependency(include_directories: inc_dirs)

subdir('tests')
