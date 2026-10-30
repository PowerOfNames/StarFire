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
layout(buffer_reference) readonly buffer GlobalStateBufferReference;
layout(buffer_reference) readonly buffer DrawDataBufferReference;
#	define AURORA_GPU_REF(T) T
#endif

#define AURORA_GPU_REF_ALIGN 8
#define AURORA_GPU_SAMPLERS_REPEAT_ID 0
#define AURORA_GPU_SAMPLER_BINDING 0
#define AURORA_GPU_IMAGE_BINDING 1

struct BindlessPushConstant
{
	AURORA_GPU_REF(GlobalStateBufferReference) GlobalStateBufferAddress;
	AURORA_GPU_REF(DrawDataBufferReference) DrawDataBufferAddress;
};

struct BindlessDrawData
{
	AURORA_GPU_REF(VertexBufferReference) VertexBufferAddress;
	AURORA_GPU_REF(MaterialBufferReference) MaterialBufferAddress;
};


struct Vertex
{
	vec3 Position;
	vec3 Color;
	vec2 TexCoord;
};


// GLSL only: the shader-side mirror of the app's globals struct (Sandbox: GlobalsData.h).
// Aurora never reads these fields; it copies the bytes the app hands to RenderGraph::Execute.
// Keep member order and types in sync with the app struct by hand - Execute checks only the size.
#ifndef __cplusplus
struct BindlessGlobalState
{
	mat4 View;
	mat4 Projection;
	mat4 ViewProjection;
	//in Seconds, wraps with 3600
	float AnimationTime;
};
#endif

/// <summary>
/// TODO: materials are different, because objects have different material compositions -> they will probably need a proper buffer abstraction that tracks where each material address is or how large the stride is
/// </summary>
struct BindlessMaterial
{
	vec3 Tint;
	uint AlbedoSlot;
};

#ifdef __cplusplus
	} // namespace
#endif
#endif