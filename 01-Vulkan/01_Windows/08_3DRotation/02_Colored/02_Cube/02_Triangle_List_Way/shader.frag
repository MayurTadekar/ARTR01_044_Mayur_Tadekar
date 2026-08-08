#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location=1) in vec4 fragColor;

layout(location=0) out vec4 FragColor;

void main(void)
{
	// Code
	FragColor = fragColor;
}
