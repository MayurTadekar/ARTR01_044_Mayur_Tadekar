#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec4 vPosition;

layout(binding=0) uniform MyUniformData
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	vec4 color;
} myUniformData;

void main(void)
{
	// Code

	gl_PointSize = 1.0;

	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
}
