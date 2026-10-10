#pragma once

#include "Aurora/BitField.h"

#include <vector>
#include <string>

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

	enum class WindingOrder
	{
		COUNTER_CLOCKWISE = 0,
		CLOCKWISE = 1,
	};

	enum class PrimitiveTopology
	{
		POINT_LIST = 0,
		LINE_LIST = 1,
		LINE_STRIP = 2,
		TRIANGLE_LIST = 3,
		TRIANGLE_STRIP = 4,
		TRIANGLE_FAN = 5,
	};

	enum class PolygonMode
	{
		FILL = 0,
		LINE = 1,
		POINT = 2,
	};

	enum class VertexInputRate
	{
		VERTEX = 0,
		INSTANCE = 1,
	};

	enum class IndexType
	{
		UINT16 = 0,
		UINT32 = 1,
		UINT8 = 1000265000,
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

	struct VertexAttribute
	{
		std::string Name;
		uint32_t Location = 0;
		FieldType Type = FieldType::U32;
		uint32_t Offset = 0; // in bytes
	};

	/// <summary>
	/// Use constructor with initializer list to create a vertex layout. The stride will be calculated automatically based on the attributes and their types.
	/// </summary>
	struct VertexLayout
	{
		std::vector<VertexAttribute> Attributes;
		uint32_t Stride = 0;

		VertexLayout(std::initializer_list<VertexAttribute> attributes)
			: Attributes(attributes)
		{
			// calculate stride
			for (const VertexAttribute& attr : Attributes)
			{
				uint32_t attrSize = 0;
				switch (attr.Type)
				{
					case FieldType::U8:
					case FieldType::I8:
						attrSize = 1;
						break;
					case FieldType::U16:
					case FieldType::I16:
						attrSize = 2;
						break;
					case FieldType::U32:
					case FieldType::I32:
					case FieldType::F32:
						attrSize = 4;
						break;
					case FieldType::U64:
					case FieldType::I64:
					case FieldType::F64:
						attrSize = 8;
						break;
					case FieldType::BOOL:
						attrSize = 1; // we can pack bools, but for simplicity we just use 1 byte per bool for now
						break;
					case FieldType::VEC2:
						attrSize = 4 * 2;
						break;
					case FieldType::VEC3:
						attrSize = 4 * 3;
						break;
					case FieldType::VEC4:
						attrSize = 4 * 4;
						break;
					case FieldType::MAT3:
						attrSize = 4 * 3 * 3; // we can pack matrices, but for simplicity we just use a flat array of floats for now
						break;
					case FieldType::MAT4:
						attrSize = 4 * 4 * 4; // we can pack matrices, but for simplicity we just use a flat array of floats for now
						break;
					default:
						break;
				}
				Stride += attrSize;
			}
		}
	};

}	

namespace Substrate {
	SST_ENABLE_BIT_OPS(Aurora::ImageUsageFlags);
	SST_ENABLE_BIT_OPS(Aurora::BufferUsageFlags);
}
