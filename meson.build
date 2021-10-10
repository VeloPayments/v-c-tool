project('vctool', 'c', 'cpp',
    version : '0.4.3-snapshot',
    default_options : ['c_std=gnu11', 'cpp_std=c++14', 'buildtype=release'],
    meson_version : '>=0.53.0'
)

message('Using build type: ' + get_option('buildtype'))

force_velo_toolchain = get_option('force_velo_toolchain')

# TODO: Move this into mesons built in warning level.
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'c')
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'cpp')

flex = find_program('flex', required : true)
bison = find_program('bison', required : true)

src_not_main = run_command(
    'find', './src', '(', '-name', '*.c', '-or', '-name', '*.h' , ')', 
            '!', '-name', 'main.c',
    check : true
).stdout().strip().split('\n')

test_src = run_command(
    'find', './test', '-name', '*.cpp', '-or', '-name', '*.h',
    check : true
).stdout().strip().split('\n')

lib_src = run_command(
    'find', './src/lib', '-name', '*.c',
    check : true
).stdout().strip().split('\n')

vcblockchain = dependency(
    'vcblockchain',
    required : true,
    fallback : ['vcblockchain', 'vcblockchain_dep']
)

vccrypt = dependency(
    'vccrypt',
    required : true,
    fallback : ['vccrypt', 'vccrypt_dep']
)

vccrypt_sub = subproject('vccrypt')
vccrypt_mock_lib = vccrypt_sub.get_variable('vccrypt_mock_lib')

cmake = import('cmake')
minunit_proj = cmake.subproject('minunit')
minunit = minunit_proj.dependency('minunit')

threads = dependency('threads')

vctool_include = include_directories('include')

vctool_exe = executable(
    'vctool',
    './src/vctool/main.c',
    src_not_main,
    include_directories : vctool_include,
    dependencies : [threads, vcblockchain]
)

vctool_test = executable(
    'vctool-test',
    src_not_main, test_src,
    include_directories : vctool_include,
    dependencies : [threads, vcblockchain, vccrypt, minunit],
    link_with : [vccrypt_mock_lib]
)

vctool_lib = static_library('vctool', lib_src,
    include_directories : vctool_include,
    dependencies : [vcblockchain]
)

vctool_dep = declare_dependency(
  link_with : vctool_lib,
  include_directories : vctool_include
)

test(
    'vctool-test',
    vctool_test,
)
