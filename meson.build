project(
    'ztl',
    'cpp',
    version: '0.1',
    default_options: ['cpp_std=c++20', 'warning_level=3'],
)

inc_dir = include_directories('include')
ztl_dep = declare_dependency(include_directories: inc_dir)

add_project_arguments('-DZTL_CONFIG_LITTLE_ENDIAN=1234', language : 'cpp')
add_project_arguments('-DZTL_CONFIG_BIG_ENDIAN=4321', language : 'cpp')

if build_machine.endian() == 'little'
  add_project_arguments('-DZTL_CONFIG_NATIVE_ENDIAN=1234', language : 'cpp')
else 
  add_project_arguments('-DZTL_CONFIG_NATIVE_ENDIAN=4321', language : 'cpp')
endif

subdir('tests')
