#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat;

out VS_OUT {
	vec2 Tex;
} vs_out;

void main(void) {
	mat4 mvMat = vMat * mMat;
	vec4 P = mvMat * vPos;
	gl_Position = pMat * P;
	vs_out.Tex = vTex;
}