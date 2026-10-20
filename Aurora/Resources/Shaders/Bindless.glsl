#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex
{
	vec3 Position;
	vec3 Color;
	vec2 TexCoord;
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer VertexReference
{
	Vertex Vertices[];
};

struct DrawData
{
	VertexReference VertexBufferAddress;
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer DrawDataReference
{
	DrawData Data[];
};

layout(push_constant) uniform BindlessPushConstant
{
	DrawDataReference DrawDataAddress;
} ddPc;