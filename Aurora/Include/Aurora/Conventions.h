#pragma once
#include "Aurora/Renderer/Types.h"

#include <glm/glm.hpp>

// ============================================================================
//  Aurora's fixed conventions.
//
//  Every camera, importer, shader and render pass is written against these
//  values. They are constants on purpose, not settings: a change only works
//  if every one of those changes with it, so editing a value here is a
//  renderer-wide change, not a preference.
//
//  Content authored in another convention (Z-up, left-handed, another unit
//  scale) is converted once, at import time. Nothing after the importer ever
//  branches on a source convention.
//
//  glm-typed constants are `inline static const`, never `constexpr`: glm
//  disables constexpr whenever SIMD is on (GLM_FORCE_INTRINSICS).
// ============================================================================

namespace Aurora {

	class Conventions
	{
	public:
		Conventions() = delete;

		// =====	Coordinate Convention	=====
		// Right-handed. +X points right, +Y points up and -Z points forward, into
		// the screen, so +Z points back towards the viewer. A camera at rest looks
		// along FORWARD with UP as its up vector.
		// In one line: cross(RIGHT, UP) == -FORWARD.
		inline static const glm::vec3 RIGHT			= glm::vec3(1.0f, 0.0f, 0.0f);
		inline static const glm::vec3 UP			= glm::vec3(0.0f, 1.0f, 0.0f);
		inline static const glm::vec3 FORWARD		= glm::vec3(0.0f, 0.0f, -1.0f);

		// =====	Clip Space and Viewport	=====
		// Vulkan's clip space has +Y pointing down. Aurora flips it once, in the
		// render-graph viewport (negative height), so clip-space +Y is up in every
		// pass. Projections must not flip it a second time (no proj[1][1] *= -1).
		// Texture UVs and gl_FragCoord keep their top-left origin: they live in
		// framebuffer space, which the flip does not touch.

		// =====	Depth Conventions		=====
		// Depth runs from 0 to 1 (glm's _ZO projections, see the check below).
		// REVERSE_Z picks the direction:
		//   false: near = 0, far = 1. Clear to 1; a nearer fragment is LESS.
		//   true:  near = 1, far = 0. Clear to 0; a nearer fragment is GREATER.
		// Reverse-Z puts float precision where perspective depth needs it most and
		// makes an infinite far plane practical. Every projection must be built to
		// match the direction: an orthographic camera swaps its near and far
		// arguments when REVERSE_Z is true.
		// Z_COMPARE is the default "nearer or equal passes" test. A pipeline may
		// pick another compare mode (EQUAL after a depth prepass, ALWAYS for UI),
		// but whatever it picks is relative to this direction.
		constexpr static bool REVERSE_Z				= true;
		constexpr static float Z_CLEAR				= REVERSE_Z ? 0.0f : 1.0f;
		constexpr static CompareMode Z_COMPARE		= REVERSE_Z ? CompareMode::GREATER_OR_EQUAL : CompareMode::LESS_OR_EQUAL;

		// =====	Winding					=====
		// Counter-clockwise triangles are front-facing, seen in the coordinate
		// convention above. The viewport flip keeps the image unmirrored, so the
		// winding you author is the winding you see.
		// Culling is not a convention: which faces a pipeline culls is its own
		// state (none for double-sided geometry, or for sprites flipped by a
		// negative scale).
		constexpr static WindingOrder FRONT_FACE	= WindingOrder::COUNTER_CLOCKWISE;


		// =====	GLM checks				=====
		// glm fixes its configuration at the first glm include of each .cpp. This
		// fails the build in any file where the project-wide define did not arrive
		// in time, instead of producing projections with the wrong depth range.
		static_assert(GLM_CONFIG_CLIP_CONTROL == GLM_CLIP_CONTROL_RH_ZO,
					  "glm must be RH_ZO: GLM_FORCE_DEPTH_ZERO_TO_ONE has to be defined project-wide.");
	};
}
