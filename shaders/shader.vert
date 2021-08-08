#version 460 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexColour;
layout(location = 2) in float vertexLuminosity;

uniform mat4 mvp;

out vec4 fragmentColour;

void main(){
    gl_Position = mvp * vertexPosition;
    fragmentColour = vertexLuminosity * vertexColour;
}