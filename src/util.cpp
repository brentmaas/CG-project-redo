#include "util.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

GLuint loadShader(const char* file, GLuint type){
    GLuint shaderId = glCreateShader(type);
    
    std::string shaderSource;
    std::ifstream shaderStream(file, std::ios::in);
    if(shaderStream.is_open()){
        std::stringstream strStream;
        strStream << shaderStream.rdbuf();
        shaderSource = strStream.str();
        shaderStream.close();
    }else{
        std::cerr << "Could not open " << file << std::endl;
        return 0;
    }
    
    const char* srcPtr = shaderSource.c_str();
    glShaderSource(shaderId, 1, &srcPtr, NULL);
    glCompileShader(shaderId);
    
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(!result){
        char errMsg[infoLogLength+1] = "";
        glGetShaderInfoLog(shaderId, infoLogLength, NULL, errMsg);
        std::cerr << "Could not compile shader " << file << ":" << std::endl;
        std::cerr << errMsg << std::endl;
        return 0;
    }
    
    return shaderId;
}

GLuint loadProgram(size_t count, const char** files, const GLuint* types){
    GLuint ids[count];
    for(size_t i = 0;i < count;++i){
        ids[i] = loadShader(files[i], types[i]);
    }
    
    GLuint programId = glCreateProgram();
    
    for(size_t i = 0;i < count;++i){
        glAttachShader(programId, ids[i]);
    }
    
    glLinkProgram(programId);
    
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(!result){
        char errMsg[infoLogLength+1] = "";
        glGetProgramInfoLog(programId, infoLogLength, NULL, errMsg);
        std::cerr << "Could not link program:" << std::endl;
        std::cerr << errMsg << std::endl;
        return 0;
    }
    
    for(size_t i = 0;i < count;++i){
        glDetachShader(programId, ids[i]);
        glDeleteShader(ids[i]);
    }
    
    return programId;
}

//Source: en.wikipedia.org/wiki/Mass%E2%80%93luminosity_relation
float luminosityFromMass(float mass){
    if(mass < 0.43f) return 0.23f * pow(mass, 2.3f);
    if(mass < 2.0f) return pow(mass, 4.0f);
    if(mass < 55.0f) return 1.4f * pow(mass, 3.5f);
    return 32000 * mass;
}

float temperatureFromMass(float mass){
    return 5772.005317f * pow(luminosityFromMass(mass) * pow(mass, 3.0f / 2.0f), 0.25f);
}

//Source: tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
void colourFromTemperature(float temp, glm::vec4& c){
    //R
    if(temp < 6600.0f){
        c.x = 1.0f;
    }else{
        c.x = 329.698727446f * pow(temp / 100.0f - 60.0f, -0.1332047592f) / 256.0f;
        if(c.x > 1.0f) c.x = 1.0f;
        if(c.x < 0.0f) c.x = 0.0f;
    }
    
    //G
    if(temp < 6600.0f){
        c.y = (99.4708025861f * log(temp / 100.0f) - 161.1195681661f) / 256.0f;
        if(c.y > 1.0f) c.y = 1.0f;
        if(c.y < 0.0f) c.y = 0.0f;
    }else{
        c.y = 288.1221695283f / 256.0f * pow(temp / 100.0f - 60.0f, -0.0755148492f);
        if(c.y > 1.0f) c.y = 1.0f;
        if(c.y < 0.0f) c.y = 0.0f;
    }
    
    //B
    if(temp < 2000.0f){
        c.z = 0.0f;
    }else if(temp > 6500.0f){
        c.z = 1.0f;
    }else{
        c.z = (138.5177312231f * log(temp / 100.0f - 10.0f) - 305.0447927307f) / 256.0f;
        if(c.z > 1.0f) c.z = 1.0f;
        if(c.z < 0.0f) c.z = 0.0f;
    }
}

glm::vec4 colourFromTemperature(float temp){
    glm::vec4 c = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    colourFromTemperature(temp, c);
    return c;
}