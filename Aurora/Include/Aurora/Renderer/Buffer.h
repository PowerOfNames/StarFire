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

		GLOBALS_BUFFER,
		DRAW_DATA_BUFFER,
		MATERIAL_BUFFER,		

		MAX_BUFFER_SPECIALIZATION_TYPE
	};

	struct BufferSpecification
	{
		std::string Name = "Buffer";
		void* Data = nullptr;
		//In bytes
		uint64_t Size = 0;
		uint64_t Offset = 0;
		BufferUsageFlags Usage = BufferUsageFlags::NONE;
		MemoryUsage MemUsage = MemoryUsage::GPU_ONLY;
		BufferSpecializationType SpecializationType = BufferSpecializationType::UNKNOWN;

		//Setting this to true will stall the CPU until the upload is finished! Use with caution.
		bool ForceImmediateUpload = false;
	};	

	struct VertexBufferSpecification : BufferSpecification
	{
		VertexBufferSpecification()
		{
			Name = "VertexBuffer";
			Usage = BufferUsageFlags::VERTEX_BUFFER;
			MemUsage = MemoryUsage::GPU_ONLY;
		}

		bool IsBindless = false;
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