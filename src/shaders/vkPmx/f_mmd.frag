#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 1, std140) uniform UBO
{
	vec3 Diffuse;
	float Alpha;
	vec3 Ambient;
	float aoStrength;
	vec3 Specular;
	float SpecularPower;
	vec3 LightColor;
	vec3 LightDir;
	//vec4 TexMulFactor;
	vec4 TexAddFactor;
	vec4 gaussianBlur;
	ivec4 TextureModes;
} ubo;

layout(set = 0, binding = 3) uniform Effect{
	float eAoStrength;
	float eBlurRadius;
	float eBlurStrength;
	float eSaturationFactor;
	float eAngle;
	float eBright;

	float eDarkPartStrength;
	int eStepOrSmoothStep;
	int eNums;
	float eBase;
	float eBaseUpper;
	float eRange;
	float eStepSize;

	float r;
	float g;
	float b;
	float a;
//	bool eSmoothStep;
	float maxSpecular;
	float specularColorStrength;
	float roughnessCoefficient;
	float specularPow;

}eff;

layout(std140, set = 0, binding = 6) uniform shadowMapConfig{
	float texelSize;
	float bias;
	float shadow;

	bool enable_pcf;
	int pcf_para;
	int pcf;

	float posCameraX;
	float posCameraY;
	float posCameraZ;
	float colorLightX;
	float colorLightY;
	float colorLightZ;



//	vec3 posCamera;
//	float pad_0;
//	vec3 colorLight;
//	float pad_1;
} shadowConfig;

layout(set = 0, binding = 2) uniform sampler2D Tex;
layout(set = 0, binding = 4) uniform sampler2D directLightShadowMap;

layout(set = 1, binding = 0) uniform sampler2D faceBlush;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in float fragDistance;
layout(location = 4) in vec4 fragPosLightSpace;

layout(location = 0) out vec4 outFragColor;

float shadowDirectionLightCalculation(vec4 fragPosLightSpace)
{
	float shadow = 0.f;

	if(!shadowConfig.enable_pcf){
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;

		float closestDepth = texture(directLightShadowMap, projCoords.xy).r;
		float currentDepth = fragPosLightSpace.z;

		shadow = currentDepth > closestDepth + shadowConfig.bias ? shadowConfig.shadow : 0.0;
	} else{
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;

		for (int x = -shadowConfig.pcf_para; x <= shadowConfig.pcf_para; x++) {
			for (int y = -shadowConfig.pcf_para; y <= shadowConfig.pcf_para; y++) {
				float pcfDepth = texture(directLightShadowMap, projCoords.xy + vec2(x, y) * shadowConfig.texelSize).r;
				shadow += fragPosLightSpace.z > pcfDepth + shadowConfig.bias ? shadowConfig.shadow : 0.0;
			}
		}

		shadow /= shadowConfig.pcf;
	}

	return shadow;
}

vec3 adjustSaturation(vec3 color, float saturationFactor) {
	float luminance = dot(color, vec3(0.299, 0.587, 0.114));
	vec3 greyScale = vec3(luminance);
	return mix(greyScale, color, saturationFactor);
}

vec3 adjustHue(vec3 color, float angle) {
	float u = cos(angle);
	float w = sin(angle);

	return vec3(
		(.299 + .701*u + .168*w) * color.r
		+ (.587 - .587*u + .330*w) * color.g
		+ (.114 - .114*u - .497*w) * color.b,
		(.299 - .299*u - .328*w) * color.r
		+ (.587 + .413*u + .035*w) * color.g
		+ (.114 - .114*u + .292*w) * color.b,
		(.299 - .3*u + 1.25*w) * color.r
		+ (.587 - .588*u - 1.05*w) * color.g
		+ (.114 + .886*u - .203*w) * color.b
	);
}

vec3 adjustValue(vec3 color, float valueFactor) {
	return color * valueFactor;
}

vec3 enhanceContrast(vec3 color) {
	float contrast = 1.3;
	color = color * contrast / (1.0 + (contrast - 1.0) * color);
	return color;
}

vec3 deepenColors(vec3 color) {
	color.b *= 1.1;
	color.r *= 1.2;
	return color;
}

vec3 extractHighLights(vec3 color, float threshold) {
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // ??????
	return brightness > threshold ? color : vec3(0.0);
}

vec3 gaussianBlur(sampler2D image, vec2 uv, float radius, vec2 resolution) {
	vec3 result = vec3(0.0);
	float total = 0.0;
	float sigma = radius / 3.0;

	for (int x = -int(radius); x <= int(radius); x++) {
		for (int y = -int(radius); y <= int(radius); y++) {
			vec2 offset = vec2(float(x), float(y)) * (1.0 / resolution);
			float weight = exp(-0.5 * (x*x + y*y) / (sigma*sigma));
			weight /= 2.0 * 3.14159265 * sigma * sigma;
			vec3 sample_g = texture(image, uv + offset).rgb;

			vec3 highLightsSample = extractHighLights(sample_g, 0.8);
			result += highLightsSample * weight;
			total += weight;
		}
	}

	return result / total;
}

void main()
{
	vec3 lightDirection = normalize(ubo.LightDir);
	vec3 ambientColor = vec3(0.15f, 0.1f, 0.1f);
	//vec3 colorLight = vec3(1.1f, 0.9f, 0.9f);
	vec3 colorLight = vec3(shadowConfig.colorLightX, shadowConfig.colorLightY, shadowConfig.colorLightZ);
	vec3 posCamera = vec3(shadowConfig.posCameraX, shadowConfig.posCameraY, shadowConfig.posCameraZ);
	float metallic = texture(Tex, inUV).r;
	float roughness = texture(Tex, inUV).g;

	vec3 color = vec3(0.0);
	float alpha = ubo.Alpha;

	float diffuse = max(dot(normalize(inNor), lightDirection), 0.f);

	float strength = 0.f;
	int nums = eff.eNums;
	float base = eff.eBase;

	if (eff.eStepOrSmoothStep == 1) {
		float baseUpper = eff.eBaseUpper;
		float range = eff.eRange;
		float stepsize_s = (baseUpper - base) / nums;
		for (int i = 0; i < nums; i++) {
			float lower = base + range * i;
			float upper = lower + range;
			strength += smoothstep(lower, upper, diffuse);
		}
	}
	if(eff.eStepOrSmoothStep == 0) {
		float stepsize_t = eff.eStepSize;
		for (int i = 0; i < nums; i++) {
			float edge = base + stepsize_t * i;
			strength += step(edge, diffuse);
		}
	}

	strength /= nums;
	diffuse = mix(eff.eDarkPartStrength, 1.f, strength);


//	float strength = smoothstep(0.45, 0.5, diffuse);
//	diffuse = mix(0.5f, 1.f, strength);


	// Ao
	float ao = texture(Tex, inUV).b;
	float Ao = mix(1.f, ao, eff.eAoStrength);

	// diff
	vec3 diffuseLight = diffuse * colorLight * Ao;

	// Ambient
	vec3 ambientLight = ambientColor * Ao;
	color = ubo.Diffuse;
	color *= (ambientLight + diffuseLight);


	/// specular
//	float maxSpecular = 0.2;
//	vec3 specColorLight = color * 0.2f;
//	vec3 viewDirection = normalize(posCamera - inPos);
//	vec3 halfDir = normalize(ubo.LightDir + viewDirection);
//	float specAngle = max(dot(halfDir, inNor), 0.0);
//	float specular = pow(specAngle, 2.0 / (roughness + 0.001)) * metallic;
//	specular = pow(specular, 1.3);
//	specular = min(specular, maxSpecular);
//	color += specular * specColorLight;

	float maxSpecular = eff.maxSpecular;
	vec3 specColorLight = color * eff.specularColorStrength;
	vec3 viewDirection = normalize(posCamera - inPos);
	vec3 halfDir = normalize(ubo.LightDir + viewDirection);
	float specAngle = max(dot(halfDir, inNor), 0.0);
	float specular = pow(specAngle, 2.0 / (roughness + eff.roughnessCoefficient)) * metallic;
	specular = pow(specular, eff.specularPow);
	specular = min(specular, maxSpecular);
	color += specular * specColorLight;


	// normalize
	color = clamp(color, vec3(0.0), vec3(1.0));
	int TexMode = ubo.TextureModes.x;

	if (TexMode != 0)
	{
		vec4 texColor = texture(Tex, inUV);

//		if (eff.eFaceblush) {
//			vec4 faceBlush = texture(faceBlush, inUV);
//			texColor = mix(texColor, texColor + faceBlush * faceBlush.a, faceBlush.a);
//		}
		color *= texColor.xyz;

		if (TexMode == 2)
		{
			alpha *= texColor.w;
		}
	}


	// blur
	vec3 original = color.rgb;
	vec3 highLights = extractHighLights(original, 0.8);

	vec2 resolution = vec2(2048);
	vec3 blurred = gaussianBlur(Tex, inUV, eff.eBlurRadius, resolution);
	color = original + blurred * eff.eBlurStrength;

	// saturation
	color = adjustSaturation(color, eff.eSaturationFactor);
	color = adjustHue(color, eff.eAngle);
	color = adjustValue(color, eff.eBright);

	color = enhanceContrast(color);
	color = deepenColors(color);

	color = clamp(color, vec3(0.0), vec3(1.0));

	if (alpha == 0.0)
	{
		discard;
	}

	color = clamp(color, vec3(0.0), vec3(1.0));

	float minDist = 0.1f;
	float maxDist = 100.0;
	float fogDensity = 3.f;
	float lightIntensity = 0.15f;
	float fogFactor = 1.0 - exp(-((fragDistance - minDist) / (maxDist - minDist)) * fogDensity);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	vec3 fogColor = vec3(0.3f, 0.f, 0.0f);
	vec3 fogEffect = mix(vec3(fogColor), color, pow(fogFactor, lightIntensity));
	color = fogEffect;

	vec3 defColor = vec3(eff.r, eff.b, eff.g);

	color = mix(color, defColor, eff.a);

	outFragColor = vec4(color, alpha);

}

