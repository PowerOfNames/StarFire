#pragma once
#include "Aurora/Renderer/Types.h"

#include <cstdint>
#include <string>

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

	struct VertexBufferSpecification : BufferSpecification
	{
		VertexBufferSpecification()
		{
			Name = "VertexBuffer";
			Usage = BufferUsageFlags::VERTEX_BUFFER;
			MemUsage = MemoryUsage::GPU_ONLY;
		}

		//TODO:
		//VertexLayout Layout{};
	};

	struct IndexBufferSpecification : BufferSpecification
	{
		IndexBufferSpecification()
		{
			Name = "IndexBuffer";
			Usage = BufferUsageFlags::INDEX_BUFFER;
			MemUsage = MemoryUsage::GPU_ONLY;
		}

		IndexType Type = IndexType::UINT16;
	};
}