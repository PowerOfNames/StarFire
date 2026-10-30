#version 460
#include "Bindless.glsl"

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat uint v_TexID;

void main()
{
	BindlessDrawData pDrawData = pcb.BindlessPC.DrawDataBufferAddress.Data[gl_InstanceIndex];
	Vertex vertex = pDrawData.VertexBufferAddress.Vertices[gl_VertexIndex];
	BindlessMaterial material = pDrawData.MaterialBufferAddress.Materials[gl_InstanceIndex];
	BindlessGlobalState globalState = pcb.BindlessPC.GlobalStateBufferAddress.Globals;

	gl_Position = globalState.ViewProjection * vec4(vertex.Position, 1.0);
	v_Color = vertex.Color;
	v_TexCoord = vertex.TexCoord;
	v_TexID = material.AlbedoSlot;
}