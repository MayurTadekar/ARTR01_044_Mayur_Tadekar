#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord;

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
} myUniformData;


layout(location = 0) out vec3 ovColor;
layout(location = 1) out vec2 ovTexCoord;

layout(location = 2) out vec3 outTransformedNormal;
layout(location = 3) out vec3 outLightDirection;
layout(location = 4) out vec3 outViewVector;

void main(void) 
{
	// Code
	
	vec4 eyeCoords = myUniformData.viewMatrix * myUniformData.modelMatrix * vec4(vPosition, 1.0);
	outTransformedNormal = mat3(myUniformData.viewMatrix * myUniformData.modelMatrix) * vNormal;
	outLightDirection = vec3(myUniformData.lightPosition - eyeCoords);
	outViewVector = -vec3(eyeCoords);

	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vec4(vPosition, 1.0);
	ovTexCoord = vTexCoord;
	ovColor = vColor;
}
