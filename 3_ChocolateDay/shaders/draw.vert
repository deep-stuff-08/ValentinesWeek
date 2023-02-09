#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
} vs_out;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat;

void main(void) {
	vs_out.fragPos = vec3(mMat * vPos);
	vs_out.normal = transpose(inverse(mat3(mMat))) * vNor;
	vs_out.texCoords = vTex;
	gl_Position = pMat * vMat * mMat * vPos;
}