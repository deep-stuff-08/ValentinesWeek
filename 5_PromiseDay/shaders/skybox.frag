#version 460 core

in vec3 tex;

layout(binding = 0)uniform samplerCube image;

layout(location = 0)out vec4 FragColor;

void main(void) {
	FragColor = texture(image, tex);
}