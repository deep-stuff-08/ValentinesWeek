#version 460 core

in vec3 pos;
in vec3 nor;

layout(binding = 0)uniform samplerCube image;

layout(location = 3)uniform vec3 cameraPos;

layout(location = 0)out vec4 FragColor;

void main(void) {
	vec3 I = vec3(pos - cameraPos);
	vec3 R;
	R = reflect(I, normalize(nor));
	FragColor = texture(image, vec3(R.x, -R.y, R.z)) * vec4(1.0, 0.7, 0.1, 1.0);
}