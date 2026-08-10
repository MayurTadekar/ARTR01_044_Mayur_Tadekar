#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0) uniform MyUniformData
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	vec4 color;
} myUniformData;

layout(location=0) out vec4 FragColor;

void main(void)
{
	// Code
	FragColor = myUniformData.color;
}
