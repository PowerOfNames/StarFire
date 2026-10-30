#include "CrudeCamera.h"
#include "Aurora/Conventions.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Sandbox {

	CrudeCamera::CrudeCamera(uint32_t width, uint32_t height, float zoomLevel /*= 1.0f*/)
		: m_Width(width), m_Height(height), m_ZoomLevel(zoomLevel)
	{
		PROFILE_FUNCTION;
		
		UpdateBounds();		
		UpdateView();
		UpdateProjection();
	}

	void CrudeCamera::OnUpdate(float deltaTimeMs)
	{
		PROFILE_FUNCTION;

		//Up
		if (StarFire::Input::IsKeyDown(StarFire::Key::W))
		{
			m_Position = glm::vec3(m_Position.x, m_Position.y + m_PanSpeed * deltaTimeMs, 0.0f);
		}
		//Left
		if (StarFire::Input::IsKeyDown(StarFire::Key::A))
		{
			m_Position = glm::vec3(m_Position.x - m_PanSpeed * deltaTimeMs, m_Position.y, 0.0f);
		}
		//Down
		if (StarFire::Input::IsKeyDown(StarFire::Key::S))
		{
			m_Position = glm::vec3(m_Position.x, m_Position.y - m_PanSpeed * deltaTimeMs, 0.0f);
		}
		//Right
		if (StarFire::Input::IsKeyDown(StarFire::Key::D))
		{
			m_Position = glm::vec3(m_Position.x + m_PanSpeed * deltaTimeMs, m_Position.y, 0.0f);
		}

		UpdateView();
	}

	void CrudeCamera::OnResize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;

		if (width == m_Width && height == m_Height)
			return;

		if (width == 0 || height == 0)
			return;

		m_Width = width;
		m_Height = height;

		UpdateBounds();
		UpdateProjection();
	}

	void CrudeCamera::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;

		StarFire::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<StarFire::MouseScrolledEvent>(STARFIRE_BIND_EVENT_FN(CrudeCamera::OnMouseScrollEvent));
	}

	bool CrudeCamera::OnMouseScrollEvent(StarFire::MouseScrolledEvent& e)
	{
		PROFILE_FUNCTION;

		m_ZoomLevel -= m_ZoomSpeed * e.GetOffsetY();
		m_ZoomLevel = glm::max(m_ZoomLevel, 0.01f);

		UpdateBounds();
		UpdateProjection();

		return false;
	}

	void CrudeCamera::UpdateBounds()
	{
		PROFILE_FUNCTION;

		float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
		m_Bounds = { -aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
	}

	void CrudeCamera::UpdateView()
	{
		PROFILE_FUNCTION;

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), -m_Position);
	}

	void CrudeCamera::UpdateProjection()
	{
		PROFILE_FUNCTION;

		m_ProjectionMatrix = Aurora::Conventions::REVERSE_Z 
			? glm::ortho(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top, m_FarClip, m_NearClip) 
			: glm::ortho(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top, m_NearClip, m_FarClip);
	}

}