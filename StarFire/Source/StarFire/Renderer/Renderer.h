#pragma once

#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/RenderGraph.h"

#include <glm/glm.hpp>

namespace StarFire {

	class RendererAPI
	{
	public:
		//static void DrawMesh(MeshHandle mesh, const glm::mat4& transform, MaterialHandle material);
		static void DrawSprite(Ref<Aurora::RenderGraph> renderGraph, Aurora::Shape2DHandle shape, const glm::mat4& transform, Aurora::ShaderHandle shader);
	};

}
