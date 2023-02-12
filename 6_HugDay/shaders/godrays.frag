#version 460 core

in vec2 texCoord;

layout(binding = 0)uniform sampler2D occlusionTexture;
layout(binding = 1)uniform sampler2D hugTexture;

layout(location = 0)uniform float density;
layout(location = 1)uniform float weight;
layout(location = 2)uniform float decay;
layout(location = 3)uniform float exposure;
layout(location = 4)uniform int numSamples;
layout(location = 5)uniform vec2 screenSpaceLightPos = vec2(0.0);

layout(location = 0)out vec4 FragColor;

void main(void) {
	vec3 fragColor = vec3(0.0,0.0,0.0);

	vec4 texColor = texture(hugTexture, texCoord);
	
	vec2 deltaTextCoord = vec2(texCoord - screenSpaceLightPos.xy);

	vec2 textCoo = texCoord.xy;
	deltaTextCoord *= (1.0 /  float(numSamples)) * density;
	float illuminationDecay = 1.0;


	for(int i=0; i < 100 ; i++){


		/*
		This makes sure that the loop only runs `numSamples` many times.
		We have to do it this way in WebGL, since you can't have a for loop
		that runs a variable number times in WebGL.
		This little hack gets around that.

		But the drawback of this is that we have to specify an upper bound to the
		number of iterations(but 100 is good enough for almost all cases.)
		*/
		if(numSamples < i) {
			break;
		}

		textCoo -= deltaTextCoord;
		vec3 samp = texture(occlusionTexture, textCoo).xyz;
		samp *= illuminationDecay * weight;
		fragColor += samp;
		illuminationDecay *= decay;
	}

	fragColor *= exposure;

	if(texColor.r + texColor.g + texColor.b < 0.1) {
		FragColor = vec4(mix(fragColor, vec3(0.0, 0.0, 0.0), 0.5), 1.0);
	} else {
		FragColor = vec4(fragColor, 1.0);
	}
}