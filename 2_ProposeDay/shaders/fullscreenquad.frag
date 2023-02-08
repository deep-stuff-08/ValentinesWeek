#version 460 core

layout(location = 0)out vec4 FragColor;

in VS_OUT {
	vec2 texCoords;
} fs_in;

layout(binding = 0)uniform sampler2D depthMap;

void main(void) {             
    float depthValue = texture(depthMap, fs_in.texCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}