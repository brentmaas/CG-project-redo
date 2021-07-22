#version 460 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexColour;

layout(location = 0) uniform mat4 mvp;

layout(location = 0) out vec4 fragmentColour;

void main(){
    gl_Position = mvp * vertexPosition;
    fragmentColour = vertexColour;
}