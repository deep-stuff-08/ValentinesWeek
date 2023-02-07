#version 460 core

layout(location = 0)out vec4 FragColor;

layout(binding = 0)uniform sampler2D diffuseTex;
layout(binding = 1)uniform sampler2D brightTex;

in vec2 tex;

layout(location = 0)uniform float fade;

void main(void) {
	FragColor = mix(texture(diffuseTex, tex) + texture(brightTex, tex), vec4(0.0, 0.0, 0.0, 1.0), fade);
	// FragColor = /*texture(diffuseTex, tex) + */texture(brightTex, tex);
}