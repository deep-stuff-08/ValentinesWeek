#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in float layer;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat;
layout(location = 5)uniform vec3 displacement;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	float layer;
} vs_out;

void main(void) {
	vec3 layerDisplacement = pow(layer, 3.0) * displacement;
	vec4 newPos = vec4(vPos.xyz + layerDisplacement, vPos.w);
	gl_Position = pMat * vMat * mMat * newPos;

	vs_out.fragPos = vec3(mMat * vPos);
	vs_out.normal = transpose(inverse(mat3(mMat))) * vNor;
	vs_out.texCoord = vTex;
	vs_out.layer = layer;
}