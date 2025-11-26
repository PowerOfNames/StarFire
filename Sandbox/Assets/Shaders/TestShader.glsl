#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_TexCoord;

layout (location = 0) out vec3 v_Color;
layout (location = 1) out vec2 v_TexCoord;

layout (std140, set = 0, binding = 0) uniform CameraData
{
	mat4 View;
	mat4 Projection;
	mat4 ViewProjection;
} u_Camera;

layout (push_constant) uniform Constants
{
	mat4 ModelMatrix;
} pc_Model;

void main()
{
	o_Color = a_Color;
	o_TexCoord = a_TexCoord;
	gl_Position = u_Camera.ViewProjection * pc_Model.ModelMatrix * vec4(a_Position, 1.0);
}

// ==========================================================================================

#type fragment
#version 450

layout (location = 0) in vec3 v_Color;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 f_Color;

void main()
{
	f_Color = vec4(v_Color * v_TexCoord.s, v_TexCoord.t);
}