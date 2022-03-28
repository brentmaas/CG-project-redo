project(
    'cgpr',
    ['c', 'cpp'],
    version: '0.0.0',
    default_options: [
        'cpp_std=c++20',
        'buildtype=debugoptimized',
    ]
)

sources = [
    'src/galaxy.cpp',
    'src/glad.c',
    'src/main.cpp',
    'src/util.cpp'
]

dependencies = [
    dependency('GL'),
    dependency('GLFW3')
]

link_args = []
if host_machine.system() == 'windows'
    link_args += ['-mwindows']
endif

run_command(find_program('cp'), '-r', join_paths(meson.source_root(), 'shaders'), meson.build_root())

executable(
    'cgpr',
    sources,
    dependencies: dependencies,
    include_directories: [include_directories('src'), include_directories('include')],
    link_args: link_args
)