#version 460 core

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	float layer;
} fs_in;

layout(binding = 0)uniform sampler2D fur;
layout(binding = 1)uniform sampler2D color;

layout(location = 3)uniform vec3 lightPos;
layout(location = 4)uniform vec3 viewPos;

layout(location = 0)out vec4 FragColor; 

void main(void) {
	vec3 normal = normalize(fs_in.normal);
	float fakeShadow = mix(0.4, 1.0, fs_in.layer);
	vec4 furData = texture(fur, fs_in.texCoord);
	vec4 furColor = texture(color, fs_in.texCoord) * fakeShadow;
	vec4 ambient = 0.3 * furColor;
	vec3 lightDir = normalize(lightPos - fs_in.fragPos);
	vec4 diffuse = max(dot(lightDir, normal), 0.0) * furColor;
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec4 specular = pow(max(dot(viewDir, reflectDir), 0.0), 64.0) * vec4(0.0);
	vec4 lighting = (ambient + diffuse + specular);

	float visibility = (fs_in.layer > furData.r) ? 0.0 : furData.a;
	furColor.a = (fs_in.layer == 0.0) ? 1.0 : visibility;
	FragColor = vec4(lighting.rgb, furColor.a);
}