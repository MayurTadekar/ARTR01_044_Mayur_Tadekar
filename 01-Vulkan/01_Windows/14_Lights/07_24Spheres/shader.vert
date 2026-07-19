#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

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

layout(location = 0) out vec3 outTransformedNormal;
layout(location = 1) out vec3 outLightDirection;
layout(location = 2) out vec3 outViewVector;

void main(void)
{
	// Code

	if(1 == myUniformData.lKeyPressed)
	{
		vec4 eyeCoords = myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
		outTransformedNormal = mat3(myUniformData.viewMatrix * myUniformData.modelMatrix) * vNormal;
		outLightDirection = vec3(myUniformData.lightPosition - eyeCoords);
		outViewVector = -vec3(eyeCoords);
	}

	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
}
