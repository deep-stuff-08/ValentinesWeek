#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTan;
layout(location = 4)in vec3 vBit;

layout(location = 0)uniform mat4 pMatrix;
layout(location = 1)uniform mat4 vMatrix;
layout(location = 2)uniform mat4 mMatrix;

layout(location = 3)uniform vec3 viewPos;
layout(location = 4)uniform vec3 lightPos;

out VS_OUT {
	vec3 fragPos;
	vec2 texCoord;
	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentFragPos;
	vec3 normals;
	vec3 lightPos;
	vec3 viewPos;
} vs_out;

void main(void) {
    vs_out.fragPos = vec3(mMatrix * vPos);   
    vs_out.texCoord = vTex;
    vec3 T = normalize(mat3(mMatrix) * vTan);
    vec3 B = normalize(mat3(mMatrix) * vBit);
    vec3 N = normalize(mat3(mMatrix) * vNor);
    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos = TBN * viewPos;
    vs_out.tangentFragPos = TBN * vs_out.fragPos;
	vs_out.normals = mat3(mMatrix) * vNor;
	vs_out.lightPos = lightPos;
	vs_out.viewPos = viewPos;
    gl_Position = pMatrix * vMatrix * mMatrix * vPos;
}