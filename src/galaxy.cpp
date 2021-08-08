#include "galaxy.hpp"

#include <iostream>

Galaxy::Galaxy(size_t n, size_t nCloud, float hr, float hz, float gmMin, float gmMax, float dt, int seed, int screenWidth, int screenHeight):
    n(n), nCloud(nCloud), hr(hr), hz(hz), totalMass(0.0f), dt(dt), salpeterA(pow(gmMin, -1.35f)), salpeterB(salpeterA - pow(gmMax, -1.35f)), salpeterC(-1.0f / 1.35f),
    randomEngine(std::default_random_engine()), distribution(std::uniform_real_distribution<float>(0, 1)) {
    
    srand(seed);
    randomEngine.seed(seed);
    
    currentPosition = std::vector<glm::vec4>(n + nCloud, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    previousPosition = std::vector<glm::vec4>(n + nCloud, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colour = std::vector<glm::vec4>(n + nCloud, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mass = std::vector<float>(n + nCloud, 1.0f);
    luminosity = std::vector<float>(n + nCloud, 1.0f);
    
    glGenBuffers(1, &currentPositionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentPositionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, currentPosition.size() * sizeof(glm::vec4), currentPosition.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &previousPositionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, previousPositionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, previousPosition.size() * sizeof(glm::vec4), previousPosition.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &massBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mass.size() * sizeof(float), mass.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &luminosityBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, luminosityBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, luminosity.size() * sizeof(float), luminosity.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &colourBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, colour.size() * sizeof(glm::vec4), colour.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &vertexScreenBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexScreenBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 24 * sizeof(float), vertexScreen, GL_STATIC_DRAW);
    
    reset();
    
    const char* hGaussShaderFiles[2] = {"shaders/gauss.vert", "shaders/hgauss.frag"};
    const GLuint hGaussShaderTypes[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    hGaussProgram = loadProgram(2, hGaussShaderFiles, hGaussShaderTypes);
    if(hGaussProgram == 0){
        std::cerr << "Could not create hGauss program" << std::endl;
    }
    hGaussImageId = glGetUniformLocation(hGaussProgram, "image");
    hGaussDepthId = glGetUniformLocation(hGaussProgram, "depth");
    
    const char* vGaussShaderFiles[2] = {"shaders/gauss.vert", "shaders/vgauss.frag"};
    const GLuint vGaussShaderTypes[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    vGaussProgram = loadProgram(2, vGaussShaderFiles, vGaussShaderTypes);
    if(vGaussProgram == 0){
        std::cerr << "Could not create vGauss program" << std::endl;
    }
    
    const char* computeShaderFiles[1] = {"shaders/verlet.comp"};
    const GLuint computeShaderTypes[1] = {GL_COMPUTE_SHADER};
    computeProgram = loadProgram(1, computeShaderFiles, computeShaderTypes);
    if(computeProgram == 0){
        std::cerr << "Could not create compute program" << std::endl;
    }
    nId = glGetUniformLocation(computeProgram, "n");
    totalGMId = glGetUniformLocation(computeProgram, "totalGM");
    dtId = glGetUniformLocation(computeProgram, "dt");
    hrId = glGetUniformLocation(computeProgram, "hr");
    hzId = glGetUniformLocation(computeProgram, "hz");
    
    glGenFramebuffers(2, framebuffers);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(2, framebufferTextures);
    glGenTextures(1, &framebufferDepth);
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
    glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextures[0], 0);
    
    GLuint framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "Could not complete first framebuffer (" << framebufferStatus << ")" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
    glBindTexture(GL_TEXTURE_2D, framebufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextures[1], 0);
    glBindTexture(GL_TEXTURE_2D, framebufferDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufferDepth, 0);
    
    framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "Could not complete second framebuffer (" << framebufferStatus << ")" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Galaxy::integrate(){
    if(computeProgram != 0){
        glUseProgram(computeProgram);
        glUniform1i(nId, n + nCloud);
        glUniform1f(totalGMId, totalMass);
        glUniform1f(dtId, dt);
        glUniform1f(hrId, hr);
        glUniform1f(hzId, hz);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, currentPositionBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, previousPositionBuffer);
        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(n + nCloud, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        std::swap(currentPositionBuffer, previousPositionBuffer);
    }
}

void Galaxy::draw(){
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, currentPositionBuffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, luminosityBuffer);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glDrawArrays(GL_POINTS, 0, n + nCloud);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    
    glUseProgram(hGaussProgram);
    glUniform1i(hGaussImageId, 0);
    glUniform1i(hGaussDepthId, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexScreenBuffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebufferDepth);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    
    glUseProgram(vGaussProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexScreenBuffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebufferTextures[1]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
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
        luminosity[i] = luminosityFromMass(m);
        
        float r2 = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        float rProj = sqrt(pos.x * pos.x + pos.y * pos.y);
        float cosTheta = pos.x * pos.x / r2 / rProj + pos.y * pos.y / r2 / rProj;
        float vTot = sqrt(totalMass * (1 - exp(-rProj / hr)) * (1 - exp(-abs(pos.z) / hz)) / r2);
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
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, luminosityBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, luminosity.size() * sizeof(float), luminosity.data());
}