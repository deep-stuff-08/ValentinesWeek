#version 460 core

layout(location = 0)out vec4 FragColor;

layout(binding = 0)uniform sampler2D textureDiffuse;
layout(binding = 1)uniform sampler2D textureNormal;
layout(binding = 2)uniform sampler2D textureDisplacement;

in VS_OUT {
	vec3 fragPos;
	vec2 texCoord;
	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentFragPos;
	vec3 normals;
	vec3 lightPos;
	vec3 viewPos;
} fs_in;

layout(location = 5)uniform float heightScale = 0.1;
layout(location = 6)uniform float mix1;
layout(location = 7)uniform float mix2;

void main(void) {
	vec3 normalsWithoutMap = normalize(fs_in.normals);
	vec3 viewDir = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);	
	vec3 viewDirWithoutMap = normalize(fs_in.viewPos - fs_in.fragPos);	
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	vec2 P = viewDir.xy / viewDir.z * heightScale; 
	vec2 deltaTexCoords = P / numLayers;
	vec2  currentTexCoords = fs_in.texCoord;
	float currentDepthMapValue = (1.0 - texture(textureDisplacement, currentTexCoords).r);
	while(currentLayerDepth < currentDepthMapValue) {
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = (1.0 - texture(textureDisplacement, currentTexCoords).r);  
		currentLayerDepth += layerDepth;  
	}
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = (1.0 - texture(textureDisplacement, prevTexCoords).r) - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
	vec2 texCoord = finalTexCoords;
	vec3 normal = texture(textureNormal, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);   
	vec3 color = texture(textureDiffuse, texCoord).rgb;
	vec3 colorWithoutMap = texture(textureDiffuse, fs_in.texCoord).rgb;
	vec3 ambient = 0.1 * color;
	vec3 lightDir = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos);
	vec3 lightDirWithoutMap = normalize(fs_in.lightPos - fs_in.fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 reflectDirWithoutMap = reflect(-lightDirWithoutMap, normalsWithoutMap);
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	vec3 halfwayDirWithoutMap = normalize(lightDirWithoutMap + viewDirWithoutMap);  
	
	vec3 diffuseWithoutMap = max(dot(lightDirWithoutMap, normalsWithoutMap), 0.0) * colorWithoutMap;
	vec3 specularWithoutMap = vec3(0.2) * pow(max(dot(normalsWithoutMap, halfwayDirWithoutMap), 0.0), 132.0);	
	vec3 diffuseWithoutBump = max(dot(lightDir, normal), 0.0) * colorWithoutMap;
	vec3 diffuse = max(dot(lightDir, normal), 0.0) * color;
	vec3 specular = vec3(0.2) * pow(max(dot(normal, halfwayDir), 0.0), 132.0);

	vec3 diffuseTexColor = ambient + diffuseWithoutMap + specularWithoutMap;
	vec3 diffuseNormalTexColor = ambient + diffuseWithoutBump + specular;
	vec3 diffuseNormalBumpTexColor = ambient + diffuse+ specular;

	FragColor = vec4(mix(mix(diffuseTexColor, diffuseNormalTexColor, clamp(mix1, 0.0, 1.0)), diffuseNormalBumpTexColor, clamp(mix2, 0.0, 1.0)), 1.0);
}