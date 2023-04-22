project(
    'Zies Template Library',
    ['cpp'],
    version: '0.0.1',
    license: 'BSL-1.0',
    default_options: ['warning_level=3'],
)

ztl_inc_dirs = include_directories('include')

ztl_dep = declare_dependency(include_directories: ztl_inc_dirs)


subdir('tests')
subdir('samples')
