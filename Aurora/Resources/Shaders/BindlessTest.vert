#version 460
#include "Bindless.glsl"
#extension GL_NV_uniform_buffer_std430_layout : enable

//layout(location = 0) in vec3 a_Position;
//layout(location = 1) in vec3 a_Color;
//layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;

struct UberVertexBuffer
{
	vec3 Position;
	vec3 Color;
	vec2 TexCoord;
};

layout(push_constant, std430) uniform pc
{
	UberVertexBuffer[] Vertices;
};

void main()
{
	UberVertex v = pc.Vertices.Vertices[gl_VertexIndex];

	gl_Position = vec4(v.Position, 1.0);
	v_Color = v.Color;
	v_TexCoord = v.TexCoord;
}