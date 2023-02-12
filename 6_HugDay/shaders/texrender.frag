#version 460 core

in vec2 texCoord;

layout(binding = 0)uniform sampler2D occlusionTexture;
layout(location = 0)uniform bool texCheck;
layout(location = 1)uniform float fade;

layout(location = 0)out vec4 FragColor;

void main(void) {
	vec4 fragcolor = texture(occlusionTexture, texCoord);
	if(texCheck) {
		if(fragcolor.r + fragcolor.g + fragcolor.b > 0.1) {
			FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		} else {
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	} else {
		FragColor = mix(fragcolor, vec4(0.0, 0.0, 0.0, 1.0), fade);
	}
}