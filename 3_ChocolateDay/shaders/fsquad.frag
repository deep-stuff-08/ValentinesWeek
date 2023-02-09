#version 460 core

layout(location = 0)out vec4 FragColor;

layout(binding = 0)uniform sampler2D diffuseTex;

in vec2 tex;

layout(location = 0)uniform float fade;

void main(void) {
	FragColor = mix(texture(diffuseTex, tex), vec4(0.0, 0.0, 0.0, 1.0), fade);
}