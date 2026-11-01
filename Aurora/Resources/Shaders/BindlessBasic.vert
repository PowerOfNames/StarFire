#version 460
#include "Bindless.glsl"

layout(location = 0) out vec3 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat uint v_TexID;

///Defined by Sandbox/App
struct AppGlobals
{
	float AnimationTime;
};

///Declared in ShaderDataTypes.h
layout(scalar, buffer_reference, buffer_reference_align = AURORA_GPU_REF_ALIGN) readonly buffer AppGlobalsBufferReference
{
	AppGlobals Globals;
};

void main()
{
	BindlessDrawData pDrawData = pcb.BindlessPC.DrawDataBufferAddress.Data[gl_InstanceIndex];
	Vertex vertex = pDrawData.VertexBufferAddress.Vertices[gl_VertexIndex];
	BindlessMaterial material = pDrawData.MaterialBufferAddress.Materials[gl_InstanceIndex];
	RendererGlobals rendGlobals = pcb.BindlessPC.RendererGlobalsBufferAddress.Globals;

	vec3 pos = vertex.Position;
	if(uvec2(pcb.BindlessPC.AppGlobalsBufferAddress) != uvec2(0))
	{
		AppGlobals appGlobals = pcb.BindlessPC.AppGlobalsBufferAddress.Globals;
		pos = vec3(pos.x * sin(appGlobals.AnimationTime), pos.y * sin(appGlobals.AnimationTime), pos.z);
	}
	gl_Position = rendGlobals.ViewProjection * vec4(pos, 1.0);
	v_Color = vertex.Color;
	v_TexCoord = vertex.TexCoord;
	v_TexID = material.AlbedoSlot;
}