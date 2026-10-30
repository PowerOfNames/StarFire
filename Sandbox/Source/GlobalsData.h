#pragma once

#include <glm/glm.hpp>

namespace Sandbox {

	// C++ side of the globals the shader reads as BindlessGlobalState (Aurora/ShaderDataTypes.h, GLSL branch).
	// Keep member order and types in sync with it by hand - RenderGraph::Execute checks only the size.
	struct GlobalsData
	{
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		glm::mat4 ViewProjectionMatrix;
		/// <summary>
		/// In Seconds. Usage wraps with SANDBOX_ANIMATION_WRAP
		/// </summary>
		float AnimationTime;
	};
}
