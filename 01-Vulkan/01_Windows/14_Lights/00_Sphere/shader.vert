#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTexCoords;


layout(binding = 0) uniform MyUniformData 
{
	// Matrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} myUniformData;

void main(void)
{
	// Code
	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
}
