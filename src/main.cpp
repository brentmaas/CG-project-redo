#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "util.hpp"

int main(){
    if(!glfwInit()){
        std::cerr << "Could not initialise GLFW" << std::endl;
        return 1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
    const int width = vidmode->width, height = vidmode->height;
    glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
    GLFWwindow* window = glfwCreateWindow(width, height, "Computer Graphics project redo", monitor, NULL);
    
    if(window == NULL){
        std::cerr << "Could not create window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(1);
    
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    
    const char* shaderFiles[2] = {"shaders/shader.vert", "shaders/shader.frag"};
    const GLuint shaderTypes[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    GLuint program = loadProgram(2, shaderFiles, shaderTypes);
    if(program == 0){
        std::cerr << "Could not create program" << std::endl;
        glfwTerminate();
        return 1;
    }
    glUseProgram(program);
    
    glm::mat4 projection = glm::perspective(70.0f, static_cast<float>(width) / height, 0.01f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 500.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;
    GLuint matrixId = glGetUniformLocation(program, "mvp");
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);
    
    glEnable(GL_MULTISAMPLE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    float vertices[12] = {-250, -125, 0, 1, 250, -125, 0, 1, 0, 250, 0, 1};
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
    
    float colours[12] = {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1};
    GLuint colourBuffer;
    glGenBuffers(1, &colourBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 12 * sizeof(float), colours, GL_STATIC_DRAW);
    
    const float cameraPanSpeed = 1.0f, cameraDragFactor = 0.002f;
    float phi = 0, theta = 0;
    double cx = 0, cy = 0;
    bool dragging = false;
    
    auto previousFrameTime = std::chrono::high_resolution_clock::now();
    
    while(!glfwWindowShouldClose(window)){
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float dt = static_cast<std::chrono::duration<float>>(currentFrameTime - previousFrameTime).count();
        previousFrameTime = currentFrameTime;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) phi += cameraPanSpeed * dt;
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) phi -= cameraPanSpeed * dt;
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) theta += cameraPanSpeed * dt;
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) theta -= cameraPanSpeed * dt;
        
        if(!dragging && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
            dragging = true;
            glfwGetCursorPos(window, &cx, &cy);
        }else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){
            dragging = false;
        }
        if(dragging){
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            phi += cameraDragFactor * (x - cx);
            theta += cameraDragFactor * (y - cy);
            cx = x;
            cy = y;
        }
        
        glm::mat4 mat = mvp * glm::rotate(glm::mat4(1.0f), theta, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), phi, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mat[0][0]);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    
    glfwTerminate();
    
    return 0;
}