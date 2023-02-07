#version 460 core

layout(binding = 0)uniform sampler2D diffuseTexture;

layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 brightColor;

in VS_OUT {
	vec2 Tex;
} fs_in;

void main(void) {
	vec4 diffuseTextureColor = texture(diffuseTexture, fs_in.Tex);
	fragColor = diffuseTextureColor;
	brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}