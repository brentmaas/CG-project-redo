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