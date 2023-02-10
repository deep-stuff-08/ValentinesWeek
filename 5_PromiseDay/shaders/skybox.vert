#version 460 core

layout(location = 0)in vec3 vPos;

layout(location = 0)uniform mat4 pMatrix;
layout(location = 1)uniform mat4 vMatrix;

out vec3 tex;

void main(void) {
	tex = vec3(vPos.x, -vPos.y, vPos.z);
	vec4 pos = pMatrix * vMatrix * vec4(vPos, 1.0);
	gl_Position = pos.xyzw;
}