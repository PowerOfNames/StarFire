#pragma once

#include "Aurora/BitField.h"

namespace Aurora {

	enum class Format
	{
		UNKNOWN		= 0,

		RGBA8_UNORM		= 37,
		RGBA8_SRGB = 43,
		DEPTH32_SFLOAT = 126,
		DEPTH24_STENCIL8 = 129,
	};

	enum class FieldType : uint8_t
	{
		UNKNOWN = 0,

		U8,
		U16,
		U32,
		U64,

		I8,
		I16,
		I32,
		I64,

		F32,
		F64,

		BOOL,

		VEC2,
		VEC3,
		VEC4,

		MAT3,
		MAT4,

		IVEC2,
		IVEC3,
		IVEC4,

		UVEC2,
		UVEC3,
		UVEC4,

		MAX_FIELD_TYPE,

		// aliases
		ID = U64,
		CHAR = U8,
		FLOAT = F32,
		DOUBLE = F64,

		RGB = VEC3,
		RGBA = VEC4,
		COLOR = RGBA,

		POSITION = VEC3,
		NORMAL = VEC3,
		TANGENT = VEC3,
		BITANGENT = VEC3,
		
		QUATERNION = VEC4,

		ROTATION = QUATERNION,
		TRANSFORM = MAT4,
	};

	enum class ImageTiling
	{
		OPTIMAL = 0,
		LINEAR = 1,
	};

	enum class ImageType
	{
		DIM_1D = 0,
		DIM_2D = 1,
		DIM_3D = 2,
	};

	enum class ImageUsageFlags : Substrate::BitField16
	{
		NONE = 0,
		TRANSFER_SRC = BIT(0),
		TRANSFER_DST = BIT(1),
		SAMPLED = BIT(2),
		STORAGE = BIT(3),
		COLOR_ATTACHMENT = BIT(4),
		DEPTH_STENCIL_ATTACHMENT = BIT(5),	
	};

	enum class MemoryUsage
	{
		UNKNOWN = 0,
		GPU_ONLY = 1,
		CPU_ONLY = 2,
		CPU_TO_GPU = 3,
		GPU_TO_CPU = 4
	};

	// these match vulkans VkBufferUsageFlagBits
	enum class BufferUsageFlags : Substrate::BitField32
	{
		NONE = 0,
		TRANSFER_SRC = BIT(0),
		TRANSFER_DST = BIT(1),
		UNIFORM_BUFFER = BIT(4),
		STORAGE_BUFFER = BIT(5),
		INDEX_BUFFER = BIT(6),
		VERTEX_BUFFER = BIT(7),
		INDIRECT_BUFFER = BIT(8),
		SHADER_DEVICE_ADDRESS = BIT(17),
	};
}

namespace Substrate {
	SST_ENABLE_BIT_OPS(Aurora::ImageUsageFlags);
	SST_ENABLE_BIT_OPS(Aurora::BufferUsageFlags);
}
