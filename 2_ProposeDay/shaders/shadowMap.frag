#version 460 core 

layout(location = 0)out vec4 FragColor;

in VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoords;
	vec4 fragPosLightSpace;
} fs_in;

layout(binding = 0)uniform sampler2D diffuse;
layout(binding = 1)uniform sampler2D shadowmap;

layout(location = 4)uniform vec3 lightPos;
layout(location = 5)uniform vec3 viewPos;

float shadowCalc(vec4 fragPosLightSpace, float bias) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	if(projCoords.z > 1.0) {
		return 0.0;
	}
    float closestDepth = texture(shadowmap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowmap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;return shadow;
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
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = shadowCalc(fs_in.fragPosLightSpace, bias);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
	FragColor = vec4(lighting, 1.0);
}