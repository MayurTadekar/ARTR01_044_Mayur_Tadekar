#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec2 ovTexCoord;

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void) {
	// Code
	FragColor = texture(uTextureSampler, ovTexCoord);
}
