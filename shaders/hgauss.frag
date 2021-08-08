#version 460 core

in vec2 texCoords;

out vec4 fragColour;

uniform sampler2D image;
uniform sampler2D depth;

//Hardcoded
uniform float near = 0.01;
uniform float far = 1000.0;
uniform float lumfactor = 40000000; //Arbitrarily set to look good

uniform float weight[10] = float[] (0.2075711176, 0.1812968389, 0.1207952457, 0.0613924796, 0.0237977808, 0.0070347137, 0.0015854863, 0.0002723888, 0.0000356633, 0.0000035575);

void main(){
	float step = 1.0 / textureSize(image, 0).x;
	float d = near + (far - near) * texture(depth, texCoords).z;
	vec3 colour = texture(image, texCoords).rgb * weight[0] / d / d / lumfactor;
	for(int i = 1;i < 10;++i){
		colour += texture(image, texCoords + vec2(step * i, 0.0)).rgb * weight[i] / d / d / lumfactor;
		colour += texture(image, texCoords - vec2(step * i, 0.0)).rgb * weight[i] / d / d / lumfactor;
	}
	fragColour = vec4(colour, 1.0);
}