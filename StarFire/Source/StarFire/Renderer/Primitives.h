#pragma once
#include "StarFire/Renderer/Vertex.h"

#include <cstdint>
#include <array>

#include <glm/glm.hpp>
namespace StarFire {

	inline constexpr std::array<Vertex, 3> TriangleVertices{ {
		{.Position = { 0.0f, 0.5f, 0.0f }, .Color = {1.0f, 0.0f, 0.0f} },
		{.Position = { 0.0f, 1.0f, 0.0f }, .Color = {0.0f, 1.0f, 0.0f} },
		{.Position = { 0.0f, 0.0f, 1.0f }, .Color = {0.0f, 0.0f, 1.0f} },
	} };
	inline constexpr std::array<uint16_t, 3> TriangleIndices = { 0, 1, 2 };
}
