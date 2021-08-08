#version 460 core

layout(location = 0) in vec4 vertexPosition;

out vec2 texCoords;

void main(){
	gl_Position = vertexPosition;
	texCoords = (vec2(1.0, 1.0) + vertexPosition.xy) / 2.0;
}