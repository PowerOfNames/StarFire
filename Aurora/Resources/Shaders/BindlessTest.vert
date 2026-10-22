#version 460
#include "Bindless.glsl"

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;

void main()
{
	BindlessDrawData pDrawData = pcb.BindlessPC.DrawDataBufferAddress.Data[gl_InstanceIndex];
	Vertex vertex = pDrawData.VertexBufferAddress.Vertices[gl_VertexIndex];
	BindlessGlobalState globalState = pcb.BindlessPC.GlobalStateBufferAddress.Globals;


	vec3 pos = vertex.Position;
	pos.x = sin(pos.x * globalState.DeltaTime * 0.1) * cos(pos.x * globalState.DeltaTime * 0.1) - 0.5;
	gl_Position = vec4(pos, 1.0);
	v_Color = vertex.Color * sin(globalState.DeltaTime * 0.01);
	v_TexCoord = vertex.TexCoord;
}