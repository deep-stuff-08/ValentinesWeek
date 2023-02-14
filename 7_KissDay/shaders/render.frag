#version 460 core

layout(location = 0)out vec4 FragColor;

in vec2 texCoord;

layout(binding = 0)uniform sampler2D texSampler;

void main(void) {
	FragColor = texture(texSampler, texCoord) * vec4(3.0, 1.0, 1.0, 1.0);
}