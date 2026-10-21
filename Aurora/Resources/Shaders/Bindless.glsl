#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#include "../../Include/Aurora/ShaderDataTypes.h"

//The rest will later move into StarFires own resources containing the bindless setup.
layout(scalar, buffer_reference, buffer_reference_align = AURORA_GPU_REF_ALIGN) readonly buffer VertexBufferReference
{
	Vertex Vertices[];
};

layout(scalar, buffer_reference, buffer_reference_align = AURORA_GPU_REF_ALIGN) readonly buffer MaterialBufferReference
{
	BindlessMaterial Materials[];
};

layout(scalar, buffer_reference, buffer_reference_align = AURORA_GPU_REF_ALIGN) readonly buffer GlobalStateBufferReference
{
	BindlessGlobalState Globals;
};

layout(scalar, buffer_reference, buffer_reference_align = AURORA_GPU_REF_ALIGN) readonly buffer DrawDataBufferReference
{
	BindlessDrawData Data[];
};

layout(push_constant, scalar) uniform PushConstantBlock
{
	BindlessPushConstant BindlessPC;
}pcb;