#version 460

layout(location = 0) in vec3 v_Color;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

void main()
{	
	o_Color = vec4(v_Color, 1.0);
}