#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0) uniform MyUniformData 
{
	mat4 mvpMatrix;

	vec4 noOfLineSegments;
	vec4 noOfLineStrips;

	vec4 lineColor;

} myUniformData;

layout(location=0) out vec4 FragColor;

void main(void)
{
	// Code
	FragColor = myUniformData.lineColor; 
}
