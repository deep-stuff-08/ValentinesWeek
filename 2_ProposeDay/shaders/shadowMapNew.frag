#version 460 core 

layout(location = 0)out vec4 FragColor;

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 fragPosLightSpace;
} fs_in;

layout(binding = 0)uniform sampler2D diffuse;
layout(binding = 1)uniform sampler2DShadow shadowmap;

layout(location = 4)uniform vec3 lightPos;
layout(location = 5)uniform vec3 viewPos;

float shadowCalc(vec4 fragPosLightSpace) {
	if(fragPosLightSpace.z > 1.0) {
		return 1.0;
	}
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowmap, 0);
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			shadow += textureProj(shadowmap, fragPosLightSpace + vec4(vec2(x, y) * texelSize, 0.0, 0.0));
		}
	}
	shadow /= 9.0;
	return shadow;
}

void main(void) {
	vec3 color = texture(diffuse, fs_in.texCoords).rgb;
	vec3 normal = normalize(fs_in.normal);
	vec3 lightColor = vec3(0.3);
	vec3 ambient = 0.3 * lightColor;
	vec3 lightDir = normalize(lightPos - fs_in.fragPos);
	vec3 diffuse = lightColor * max(dot(lightDir, normal), 0.0);
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 64.0) * lightColor;
	float shadow = shadowCalc(fs_in.fragPosLightSpace);
	vec3 lighting = (ambient + shadow * (diffuse + specular)) * color;
	FragColor = vec4(lighting, 1.0);
}