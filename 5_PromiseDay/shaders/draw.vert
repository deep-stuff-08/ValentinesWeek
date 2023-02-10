#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in ivec4 boneIds; 
layout(location = 3)in vec4 weights;

layout(location = 0)uniform mat4 pMatrix;
layout(location = 1)uniform mat4 vMatrix;
layout(location = 2)uniform mat4 mMatrix;
layout(location = 4)uniform bool isDynamic;
layout(location = 6)uniform mat4 finalBonesMatrices[100];

out vec3 pos;
out vec3 nor;

void main(void) {
	if(isDynamic) {
		vec4 totalPosition = vec4(0.0);
		vec3 totalNormal = vec3(0.0);
		for(int i = 0 ; i < 4; i++) {
			if(boneIds[i] == -1) {
				continue;
			}
			vec4 localPosition = finalBonesMatrices[boneIds[i]] * vPos;
			totalPosition += localPosition * weights[i];
			vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * vNor;
			totalNormal += localNormal;
		}
		
		mat4 viewModel = vMatrix * mMatrix;
		gl_Position =  pMatrix * viewModel * totalPosition;
		pos = (mMatrix * totalPosition).xyz;
		nor = normalize(mat3(mMatrix) * totalNormal);
	} else {
		pos = vec3(mMatrix * vPos);
		nor = mat3(mMatrix) * vNor;
		gl_Position = pMatrix * vMatrix * mMatrix * vPos;
	}
}