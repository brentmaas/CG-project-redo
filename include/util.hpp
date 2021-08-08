#ifndef UTIL_HPP
#define UTIL_HPP

#define _USE_MATH_DEFINES
#include <cmath>
#include <glad/glad.h>

GLuint loadProgram(size_t count, const char** files, const GLuint* types);

float luminosityFromMass(float mass);

#endif