#version 460 core

layout(location = 0) in vec4 fragmentColour;

layout(location = 0) out vec4 colour;

void main(){
    colour = fragmentColour;
}