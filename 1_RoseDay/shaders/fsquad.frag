#version 460 core

layout(location = 0)out vec4 FragColor;

layout(binding = 0)uniform sampler2D diffuseTex;
layout(binding = 1)uniform sampler2D brightTex;

in vec2 tex;

void main(void) {
	FragColor = texture(diffuseTex, tex) + texture(brightTex, tex);
	// FragColor = /*texture(diffuseTex, tex) + */texture(brightTex, tex);
}