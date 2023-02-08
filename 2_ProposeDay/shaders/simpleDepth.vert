#version 460 core

layout(location = 0)in vec4 vPos;

layout(location = 0)uniform mat4 lsMat;
layout(location = 2)uniform mat4 mMat;

void main(void) {
	gl_Position = lsMat * mMat * vPos;
}