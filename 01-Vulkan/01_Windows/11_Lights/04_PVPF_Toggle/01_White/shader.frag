#version 450 core

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 outTransformedNormal;
layout(location = 1) in vec3 outLightDirection;
layout(location = 2) in vec3 outViewVector;
layout(location = 3) in vec3 outPhongADS;

layout(binding = 0) uniform MyUniformData 
{
	// Matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// Lighting
	vec4 lightAmbient;
	vec4 lightDiffuse;
	vec4 lightSpecular;
	vec4 lightPosition;

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

	if(1 == myUniformData.lKeyPressed)
	{
		phongAdsLight = outPhongADS;
	}
	else if(2 == myUniformData.lKeyPressed)
	{
		vec3 _transformedNormal = normalize(outTransformedNormal);
		vec3 _lightDirection = normalize(outLightDirection);
		vec3 _viewVector = normalize(outViewVector);

		vec3 reflectionVector = reflect(-_lightDirection, _transformedNormal);
		vec3 intensityAmbient = myUniformData.lightAmbient.rgb * myUniformData.materialAmbient.rgb;
		vec3 intensityDiffuse = myUniformData.lightDiffuse.rgb * myUniformData.materialDiffuse.rgb * max(0.0, dot(_transformedNormal, _lightDirection));
		vec3 intensitySpecular = myUniformData.lightSpecular.rgb * myUniformData.materialSpecular.rgb * pow(max(0.0, dot(reflectionVector, _viewVector)), myUniformData.materialShininess);

		phongAdsLight = intensityAmbient + intensityDiffuse + intensitySpecular;
	}
	else
	{
		phongAdsLight = vec3(1.0);
	}

	FragColor = vec4(phongAdsLight, 1.0);
}
