#pragma once

#include <Aurora/Aurora.h>

#include <string>
#include <cstdint>
#include <glm/glm.hpp>

namespace StarFire {

	struct IdComponent
	{
		uint64_t Id = 0;
	};

	struct TagComponent
	{
		std::string Tag = "TagComponent";
	};

	struct SpriteComponent
	{
		// #TODO: AuroraHandles are not serializable, as they change in between runtimes. They should be replaced with a StarFire AsetHandle thatsi serializable and can be used to retrieve the respective resources at runtime.
		Aurora::VertexBufferHandle VertexBufferHandle = Aurora::VertexBufferHandle::INVALID_HANDLE;
		Aurora::IndexBufferHandle IndexBufferHandle = Aurora::IndexBufferHandle::INVALID_HANDLE;
		Aurora::ImageHandle TextureHandle = Aurora::ImageHandle::INVALID_HANDLE;
	};
}
