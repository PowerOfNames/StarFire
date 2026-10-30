#version 460
#include "Bindless.glsl"

layout(location = 0) in vec3 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat uint v_TexID;


layout(location = 0) out vec4 o_Color;

void main()
{		
	o_Color = texture(sampler2D(u_Textures[nonuniformEXT(v_TexID)], u_Samplers[AURORA_GPU_SAMPLERS_REPEAT_ID]), v_TexCoord * 8.0);
}