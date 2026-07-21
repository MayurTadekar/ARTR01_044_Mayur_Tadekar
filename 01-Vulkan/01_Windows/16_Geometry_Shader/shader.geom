#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;

layout(binding=0) uniform MyUniformData 
{
	mat4 mvpMatrix;
} myUniformData;

layout(triangle_strip, max_vertices = 9) out;

void main(void)
{
	// Code
	for(int le = 0; le < 3; ++le)
	{
		gl_Position = myUniformData.mvpMatrix * (gl_in[le].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		EmitVertex();
		
		gl_Position = myUniformData.mvpMatrix * (gl_in[le].gl_Position + vec4(-1.0, -1.0, 0.0, 0.0));
		EmitVertex();

		gl_Position = myUniformData.mvpMatrix * (gl_in[le].gl_Position + vec4(1.0, -1.0, 0.0, 0.0));
		EmitVertex();

		EndPrimitive();
	}
}
