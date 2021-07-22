#include "util.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

GLuint ShaderBinary::load() const{
    GLuint shaderId = glCreateShader(this->type);
    glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, this->binary, this->length);
    glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);

    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(!result){
        char errMsg[infoLogLength+1];
        glGetShaderInfoLog(shaderId, infoLogLength, NULL, errMsg);
        std::cerr << "Could not compile shader:" << std::endl;
        std::cerr << errMsg << std::endl;
        return 0;
    }

    return shaderId;
}

GLuint loadProgram(size_t count, const ShaderBinary* binaries){
    GLuint ids[count];
    for(size_t i = 0;i < count;++i){
        ids[i] = binaries[i].load();
        if (ids[i] == 0)
            return 0;
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
        char errMsg[infoLogLength+1];
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
