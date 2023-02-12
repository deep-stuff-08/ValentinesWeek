#version 460 core

layout(location = 0)out vec4 FragColor;

layout(location = 3)uniform vec4 color;

void main(void) {
	FragColor = color;
}