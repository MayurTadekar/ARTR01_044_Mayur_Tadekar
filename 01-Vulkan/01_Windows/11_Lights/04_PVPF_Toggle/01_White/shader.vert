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

	// Material
	vec4 materialAmbient;
	vec4 materialDiffuse;
	vec4 materialSpecular;
	float materialShininess;

	// Key Pressed
	uint lKeyPressed;
} myUniformData;

layout(location = 0) out vec3 outTransformedNormal;
layout(location = 1) out vec3 outLightDirection;
layout(location = 2) out vec3 outViewVector;
layout(location = 3) out vec3 outPhongADS;

void main(void)
{
	// Code

	if(1 == myUniformData.lKeyPressed)
	{
		vec4 eyeCoords = myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
		vec3 transformedNormal = normalize(mat3(myUniformData.viewMatrix * myUniformData.modelMatrix) * vNormal);
		vec3 lightDirection = normalize(vec3(myUniformData.lightPosition - eyeCoords));
		vec3 viewVector = normalize(-vec3(eyeCoords));
	
		vec3 reflectVector = reflect(-lightDirection, transformedNormal);
		vec3 intensityAmbient = myUniformData.lightAmbient.rgb * myUniformData.materialAmbient.rgb;
		vec3 intensityDiffuse = myUniformData.lightDiffuse.rgb * myUniformData.materialDiffuse.rgb * max(dot(transformedNormal, lightDirection), 0.0);
		vec3 intensitySpecular = myUniformData.lightSpecular.rgb * myUniformData.materialSpecular.rgb * pow(max(dot(reflectVector, viewVector), 0.0), myUniformData.materialShininess);

		outPhongADS = intensityAmbient + intensityDiffuse + intensitySpecular;
	}
	else if(2 == myUniformData.lKeyPressed)
	{
		vec4 eyeCoords = myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
		outTransformedNormal = mat3(myUniformData.viewMatrix * myUniformData.modelMatrix) * vNormal;
		outLightDirection = vec3(myUniformData.lightPosition - eyeCoords);
		outViewVector = -vec3(eyeCoords);
	}
	else
	{
		outPhongADS = vec3(1.0);
	}

	gl_Position = myUniformData.projectionMatrix * myUniformData.viewMatrix * myUniformData.modelMatrix * vPosition;
}
