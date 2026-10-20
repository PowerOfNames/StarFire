#version 460
#include "Bindless.glsl"

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;

void main()
{
	restrict DrawData pDrawData = ddPc.DrawDataAddress.Data[gl_InstanceIndex];
	Vertex pVertex = pDrawData.VertexBufferAddress.Vertices[gl_VertexIndex];

	gl_Position = vec4(pVertex.Position, 1.0);
	v_Color = pVertex.Color;
	v_TexCoord = pVertex.TexCoord;
}