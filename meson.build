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
    'src/glad.c',
   'src/main.cpp',
   'src/util.cpp'
]

link_args = []
if host_machine.system() == 'windows'
    link_args += ['-mwindows']
endif

executable(
    'cgpr',
    sources,
    dependencies: [dependency('GL'), dependency('GLFW3')],
    include_directories: [include_directories('src'), include_directories('include')],
    link_args: link_args
)