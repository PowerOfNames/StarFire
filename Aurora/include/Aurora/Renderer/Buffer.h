#pragma once
#include "Aurora/Renderer/Types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Aurora {

	enum class BufferSpecializationType : uint8_t
	{
		UNKNOWN = 0,

		STATIC_VERTEX_BUFFER,
		DYNAMIC_VERTEX_BUFFER,

		STATIC_INDEX_BUFFER,
		DYNAMIC_INDEX_BUFFER,

		STATIC_TRANSFORM_DATA_BUFFER,
		DYNAMIC_TRANSFORM_DATA_BUFFER,

		GLOBAL_DESCRIPTORS_SSBO,
		CAMERA_DATA_BUFFER = GLOBAL_DESCRIPTORS_SSBO,
		LIGHT_DATA_BUFFER = GLOBAL_DESCRIPTORS_SSBO,

		STATIC_DRAW_DATA_BUFFER,
		DYNAMIC_DRAW_DATA_BUFFER,

		STATIC_MATERIAL_DATA_BUFFER,
		DYNAMIC_MATERIAL_DATA_BUFFER,


		MAX_BUFFER_SPECIALIZATION_TYPE
	};

	struct BufferSpecification
	{
		std::string Name = "Buffer";
		void* Data = nullptr;
		uint64_t Size = 0;
		BufferUsageFlags Usage = BufferUsageFlags::NONE;
		MemoryUsage MemUsage = MemoryUsage::GPU_ONLY;
		BufferSpecializationType SpecializationType = BufferSpecializationType::UNKNOWN;
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

	struct VertexBufferSpecification : BufferSpecification
	{
		VertexBufferSpecification()
		{
			Name = "VertexBuffer";
			Usage = BufferUsageFlags::VERTEX_BUFFER;
			MemUsage = MemoryUsage::GPU_ONLY;
		}

		VertexLayout Layout{};
	};

	struct IndexBufferSpecification : BufferSpecification
	{
		IndexBufferSpecification()
		{
			Name = "IndexBuffer";
			Usage = BufferUsageFlags::INDEX_BUFFER;
			MemUsage = MemoryUsage::GPU_ONLY;
		}

		uint32_t IndexCount = 0;
		FieldType IndexType = FieldType::U16;
	};
}