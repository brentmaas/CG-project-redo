project(
    'cgpr',
    ['c', 'cpp'],
    version: '0.0.0',
    default_options: [
        'cpp_std=c++20',
        'buildtype=debugoptimized',
    ],
)

sources = [
    'src/glad.c',
    'src/main.cpp',
    'src/util.cpp',
    'src/galaxy.cpp',
]

shaders = [
    'shaders/shader.frag',
    'shaders/shader.vert',
    'shaders/verlet.comp',
]

dependencies = [
    dependency('GL'),
    dependency('glfw3'),
    dependency('glm'),
]

glslang_validator = find_program('glslangValidator')
glsl_gen = generator(
    glslang_validator,
    output: '@PLAINNAME@.h',
    arguments: ['-G', '-o', '@OUTPUT@', '--quiet', '-g', '@INPUT@', '@EXTRA_ARGS@']
)

shader_headers = []
foreach shader : shaders
    shader_name = shader.underscorify()
    shader_headers += glsl_gen.process(shader, extra_args: ['--vn', shader_name])
endforeach

link_args = []
if host_machine.system() == 'windows'
    link_args += ['-mwindows']
elif host_machine.system() == 'linux'
    dependencies += meson.get_compiler('c').find_library('dl', required: false)
endif

executable(
    'cgpr',
    [shader_headers, sources],
    dependencies: dependencies,
    include_directories: [include_directories('src'), include_directories('include')],
    link_args: link_args,
)
