#include "galaxy.hpp"
#include "verlet.comp.h"

#include <iostream>

const GLuint VERLET_N_LOCATION = 0;
const GLuint VERLET_TOTAL_GM_LOCATION = 1;
const GLuint VERLET_DT_LOCATION = 2;
const GLuint VERLET_HR_LOCATION = 3;
const GLuint VERLET_HZ_LOCATION = 4;

Galaxy::Galaxy(size_t n, size_t nCloud, float hr, float hz, float gmMin, float gmMax, float dt, int seed):
    n(n), nCloud(nCloud), hr(hr), hz(hz), totalMass(0.0f), dt(dt), salpeterA(pow(gmMin, -1.35f)), salpeterB(salpeterA - pow(gmMax, -1.35f)), salpeterC(-1.0f / 1.35f),
    randomEngine(std::default_random_engine()), distribution(std::uniform_real_distribution<float>(0, 1)) {
    
    srand(seed);
    randomEngine.seed(seed);
    
    currentPosition = std::vector<glm::vec4>(n + nCloud, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    previousPosition = std::vector<glm::vec4>(n + nCloud, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colour = std::vector<glm::vec4>(n + nCloud, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mass = std::vector<float>(n + nCloud, 1.0f);
    
    glGenBuffers(1, &currentPositionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentPositionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, currentPosition.size() * sizeof(glm::vec4), currentPosition.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &previousPositionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, previousPositionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, previousPosition.size() * sizeof(glm::vec4), previousPosition.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &massBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mass.size() * sizeof(float), mass.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &colourBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colour.size() * sizeof(glm::vec4), colour.data(), GL_STATIC_DRAW);
    
    reset();
    
    ShaderBinary shader = {shaders_verlet_comp, sizeof(shaders_verlet_comp), GL_COMPUTE_SHADER};
    computeProgram = loadProgram(1, &shader);
    if(computeProgram == 0){
        std::cerr << "Could not create compute program" << std::endl;
    }
}

void Galaxy::integrate(){
    if(computeProgram != 0){
        glUseProgram(computeProgram);
        glUniform1i(VERLET_N_LOCATION, n + nCloud);
        glUniform1f(VERLET_TOTAL_GM_LOCATION, totalMass);
        glUniform1f(VERLET_DT_LOCATION, dt);
        glUniform1f(VERLET_HR_LOCATION, hr);
        glUniform1f(VERLET_HZ_LOCATION, hz);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, currentPositionBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, previousPositionBuffer);
        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(n + nCloud, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        std::swap(currentPositionBuffer, previousPositionBuffer);
    }
}

void Galaxy::draw(){
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, currentPositionBuffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glDrawArrays(GL_POINTS, 0, n + nCloud);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Galaxy::reset(){
    totalMass = 0.0f;
    for(size_t i = 0;i < n + nCloud;++i){
        glm::vec4& pos = currentPosition[i];
        glm::vec4& prevPos = previousPosition[i];
        
        float dr = distribution(randomEngine);
        float dz = distribution(randomEngine);
        float phi = 2 * M_PI * distribution(randomEngine);
        float r = -hr * log(1 - dr);
        pos.x = r * cos(phi);
        pos.y = r * sin(phi);
        if(dz <= 0.5f) pos.z = -hz * log(1 - 2 * dz);
        else pos.z = hz * log(2 * dz - 1);
        
        float& m = mass[i];
        m = pow(salpeterA - salpeterB * distribution(randomEngine), salpeterC);
        totalMass += m;
        
        float r2 = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        float rProj = sqrt(pos.x * pos.x + pos.y * pos.y);
        float cosTheta = pos.x * pos.x / r2 / rProj + pos.y * pos.y / r2 / rProj;
        float vTot = sqrt(totalMass * (1 - exp(-rProj / hr)) * (1 - exp(-std::abs(pos.z) / hz)) / r2);
        float vProj = vTot * cosTheta;
        prevPos.x = pos.x - vProj * pos.y / rProj * dt;
        prevPos.y = pos.y + vProj * pos.x / rProj * dt;
        prevPos.z = pos.z - ((pos.z > 0) - (pos.z < 0)) * vTot * sqrt(1 - cosTheta * cosTheta) * dt;
    }
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentPositionBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, currentPosition.size() * sizeof(glm::vec4), currentPosition.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, previousPositionBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, previousPosition.size() * sizeof(glm::vec4), previousPosition.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, mass.size() * sizeof(float), mass.data());
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, colour.size() * sizeof(glm::vec4), colour.data());
}