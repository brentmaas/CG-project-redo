#ifndef UTIL_HPP
#define UTIL_HPP

#include <glad/glad.h>

#include <cstddef>

struct ShaderBinary {
    const GLuint* binary;
    const GLsizei length;
    const GLuint type;

    GLuint load() const;
};

GLuint loadProgram(size_t count, const ShaderBinary* binaries);

#endif