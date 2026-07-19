#version 450 core

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 outTransformedNormal;
layout(location = 1) in vec3 outLightDirection;
layout(location = 2) in vec3 outViewVector;

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

	// Key Pressed
	uint lKeyPressed;
} myUniformData;

layout (push_constant) uniform PushConstantData_Material
{
    // Material Related
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
    float materialShininess;
}pushConstantData_Material;

layout(location=0) out vec4 FragColor;

vec3 phongAdsLight;

void main(void)
{
	// Code

	if(1 == myUniformData.lKeyPressed)
	{
		vec3 _transformedNormal = normalize(outTransformedNormal);
		vec3 _lightDirection = normalize(outLightDirection);
		vec3 _viewVector = normalize(outViewVector);

		vec3 reflectionVector = reflect(-_lightDirection, _transformedNormal);
		vec3 intensityAmbient = myUniformData.lightAmbient.rgb * pushConstantData_Material.materialAmbient.rgb;
		vec3 intensityDiffuse = myUniformData.lightDiffuse.rgb * pushConstantData_Material.materialDiffuse.rgb * max(0.0, dot(_transformedNormal, _lightDirection));
		vec3 intensitySpecular = myUniformData.lightSpecular.rgb * pushConstantData_Material.materialSpecular.rgb * pow(max(0.0, dot(reflectionVector, _viewVector)), pushConstantData_Material.materialShininess);

		phongAdsLight = intensityAmbient + intensityDiffuse + intensitySpecular;
	}
	else
	{
		phongAdsLight = vec3(1.0);
	}

	FragColor = vec4(phongAdsLight, 1.0);
}
