#version 450 core

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 outTransformedNormal[3];
layout(location = 3) in vec3 outLightDirection[3];
layout(location = 6) in vec3 outViewVector[3];
layout(location = 9) in vec3 outPhongADS;

layout(binding = 0) uniform MyUniformData 
{
	// Matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// Lighting
	vec4 lightAmbient[3];
	vec4 lightDiffuse[3];
	vec4 lightSpecular[3];
	vec4 lightPosition[3];

	// Material
	vec4 materialAmbient;
	vec4 materialDiffuse;
	vec4 materialSpecular;
	float materialShininess;

	// Key Pressed
	uint lKeyPressed;
} myUniformData;

layout(location=0) out vec4 FragColor;

vec3 phongAdsLight;

void main(void)
{
	// Code

	phongAdsLight = vec3(0.0);

	if(1 == myUniformData.lKeyPressed)
	{
		phongAdsLight = outPhongADS;
	}
	else if(2 == myUniformData.lKeyPressed)
	{
		for(int le = 0; le < 3; ++le)
		{
			vec3 _transformedNormal = normalize(outTransformedNormal[le]);
			vec3 _lightDirection = normalize(outLightDirection[le]);
			vec3 _viewVector = normalize(outViewVector[le]);

			vec3 reflectionVector = reflect(-_lightDirection, _transformedNormal);
			vec3 intensityAmbient = myUniformData.lightAmbient[le].rgb * myUniformData.materialAmbient.rgb;
			vec3 intensityDiffuse = myUniformData.lightDiffuse[le].rgb * myUniformData.materialDiffuse.rgb * max(0.0, dot(_transformedNormal, _lightDirection));
			vec3 intensitySpecular = myUniformData.lightSpecular[le].rgb * myUniformData.materialSpecular.rgb * pow(max(0.0, dot(reflectionVector, _viewVector)), myUniformData.materialShininess);

			phongAdsLight += intensityAmbient + intensityDiffuse + intensitySpecular;
		}
	}
	else
	{
		phongAdsLight = vec3(1.0);
	}

	FragColor = vec4(phongAdsLight, 1.0);
}
