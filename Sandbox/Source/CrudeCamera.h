#pragma once
#include "StarFire.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace Sandbox {

	struct OrthoBounds
	{
		float Left, Right, Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class CrudeCamera
	{
	public:
		CrudeCamera(uint32_t width, uint32_t height, float zoomLevel = 1.0f, float nearClip = -1.0f, float farClip = 1.0f);
		~CrudeCamera() = default;

		void OnUpdate(float deltaTimeMs);
		void OnResize(uint32_t width, uint32_t height);
		void OnEvent(StarFire::Event& e);

		inline void SetPanSpeed(float speed) { m_PanSpeed = speed; }

		inline const glm::mat4& GetView() const { return m_ViewMatrix; }
		inline const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		inline const glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

	private:
		bool OnKeyPressedEvent(StarFire::KeyPressedEvent& e);
		bool OnKeyReleasedEvent(StarFire::KeyReleasedEvent& e);
		bool OnMouseScrollEvent(StarFire::MouseScrolledEvent& e);
		bool OnMousePressEvent(StarFire::MousePressEvent& e);

		void UpdateBounds();
		void UpdateView();
		void UpdateProjection();
	private:

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		OrthoBounds m_Bounds;

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::vec3 m_Position = glm::vec3(0.0f);


		float m_NearClip = 0.0f;
		float m_FarClip = 1.0f;

		float m_PanSpeed = 0.001f;
		float m_ZoomLevel = 1.0f;
		float m_ZoomSpeed = 0.1f;
	};

}