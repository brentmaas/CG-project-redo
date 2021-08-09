#ifndef GALAXY_HPP
#define GALAXY_HPP

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <random>
#include <glm/glm.hpp>

#include "util.hpp"

class Galaxy {
public:
    Galaxy(size_t n, size_t nCloud, float hr, float hz, float gmMin, float gmMax, float dt, int seed, int screenWidth, int screenHeight);
    void integrate();
    void draw();
    void reset();
private:
    size_t n, nCloud;
    float hr, hz, totalMass, dt;
    float salpeterA, salpeterB, salpeterC;
    std::vector<glm::vec4> currentPosition, previousPosition, colour;
    std::vector<float> mass, luminosity, temperature;
    const float vertexScreen[24] = {-1, -1, 1, 1, -1, 1, 1, 1, 1, -1, 1, 1, -1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1};
    GLuint hGaussProgram, vGaussProgram, computeProgram;
    GLuint nId, totalGMId, dtId, hrId, hzId;
    GLuint currentPositionBuffer, previousPositionBuffer, massBuffer, colourBuffer, luminosityBuffer, vertexScreenBuffer;
    GLuint framebuffers[2];
    GLuint framebufferTextures[2];
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<float> distribution;
};

#endif