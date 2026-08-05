#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(binding = 0) uniform MyUniformData 
{
	// Matrices Related Uniforms
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	// Lighting Related Uniform
	vec4 lightDiffuse;
	vec4 lightPosition;

	vec4 materialDiffuse;

	// Keypressed Related Uniform
	uint lKeyPressed;
} myUniformData;

layout(location = 0) out vec3 oDiffuseColor;

void main(void)
{
	// Code
	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
	
	if(1 == myUniformData.lKeyPressed)
	{
		vec4 eyeCoordinates = myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
		mat3 normalMatrix = mat3(transpose(inverse(myUniformData.viewMatrix * myUniformData.modelMatrix)));
		vec3 transformedNormal = normalize(normalMatrix * vNormal);
		vec3 lightSource = normalize(vec3(myUniformData.lightPosition - eyeCoordinates));

		oDiffuseColor = myUniformData.materialDiffuse.rgb * myUniformData.lightDiffuse.rgb * max(dot(lightSource, transformedNormal), 0.0);
	}
	else
	{
		oDiffuseColor = vec3(1.0);
	}
}
