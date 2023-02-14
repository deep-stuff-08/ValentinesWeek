#version 460 core

layout(location = 0)in vec2 vPos;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat;

out vec2 texCoord;

void main(void) {
	gl_Position = pMat * vMat * mMat * vec4(vPos, 0.0, 1.0);
	texCoord = clamp(vPos, 0.0, 1.0);
}