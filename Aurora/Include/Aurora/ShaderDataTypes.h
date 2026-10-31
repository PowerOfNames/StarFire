#ifndef AURORA_SHADER_DATA_TYPES_H
#define AURORA_SHADER_DATA_TYPES_H

#ifdef __cplusplus
#	include <cstdint>
#	include <glm/glm.hpp>
#	define AURORA_GPU_REF(T) uint64_t
namespace Aurora::GPU {
	using vec2 = glm::vec2; 
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;
	using mat3 = glm::mat3;
	using mat4 = glm::mat4;
	using uint = uint32_t;
#else
layout(buffer_reference) readonly buffer VertexBufferReference;
layout(buffer_reference) readonly buffer MaterialBufferReference;
layout(buffer_reference) readonly buffer RendererGlobalsBufferReference;
layout(buffer_reference) readonly buffer DrawDataBufferReference;

/// <summary>
/// Needs to be defined by user shader code.
/// </summary>
layout(buffer_reference) readonly buffer AppGlobalsBufferReference;
#	define AURORA_GPU_REF(T) T
#endif

#define AURORA_GPU_REF_ALIGN 8
#define AURORA_GPU_SAMPLERS_REPEAT_ID 0
#define AURORA_GPU_SAMPLER_BINDING 0
#define AURORA_GPU_IMAGE_BINDING 1

struct BindlessPushConstant
{
	AURORA_GPU_REF(RendererGlobalsBufferReference) RendererGlobalsBufferAddress;
	AURORA_GPU_REF(AppGlobalsBufferReference) AppGlobalsBufferAddress;
	AURORA_GPU_REF(DrawDataBufferReference) DrawDataBufferAddress;
};

struct BindlessDrawData
{
	AURORA_GPU_REF(VertexBufferReference) VertexBufferAddress;
	AURORA_GPU_REF(MaterialBufferReference) MaterialBufferAddress;
};

struct RendererGlobals
{
	mat4 View;
	mat4 Projection;
	mat4 ViewProjection;
};

/// <summary>
/// TODO: materials are different, because objects have different material compositions -> they will probably need a proper buffer abstraction that tracks where each material address is or how large the stride is
/// </summary>
struct BindlessMaterial
{
	vec3 Tint;
	uint AlbedoSlot;
};

struct Vertex
{
	vec3 Position;
	vec3 Color;
	vec2 TexCoord;
};

#ifdef __cplusplus
	static_assert(sizeof(BindlessPushConstant) == 24, "Must match GLSL scalar layout");
	static_assert(sizeof(BindlessDrawData) == 16, "Must match GLSL scalar layout");
	static_assert(sizeof(RendererGlobals) == 192, "Must match GLSL scalar layout");
	static_assert(sizeof(BindlessMaterial) == 16, "Must match GLSL scalar layout");
	static_assert(sizeof(Vertex) == 32, "Must match GLSL scalar layout");
#endif

#ifdef __cplusplus
	} // namespace
#endif
#endif