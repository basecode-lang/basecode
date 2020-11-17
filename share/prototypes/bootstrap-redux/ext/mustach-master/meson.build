project('mustach', 'c',
    version: '1.0.0'
)

mustach_inc = include_directories('.')
mustach_lib = shared_library('mustach',
    'mustach.c',
    include_directories: mustach_inc
)

mustach_dep = declare_dependency(link_with: mustach_lib,
    include_directories: mustach_inc)
