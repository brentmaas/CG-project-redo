#ifndef UTIL_HPP
#define UTIL_HPP

#define _USE_MATH_DEFINES
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>

GLuint loadProgram(size_t count, const char** files, const GLuint* types);

float luminosityFromMass(float mass);
float temperatureFromMass(float mass);
void colourFromTemperature(float temp, glm::vec4& c);
glm::vec4 colourFromTemperature(float temp);

#endif