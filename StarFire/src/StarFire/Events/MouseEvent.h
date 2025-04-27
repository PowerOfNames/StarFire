#pragma once
#include "StarFire/Events/Event.h"

#include <sstream>

namespace StarFire {

	class MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y)
		{
		}
		inline float GetMouseX() const { return m_MouseX; }
		inline float GetMouseY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: (" << m_MouseX << ", " << m_MouseY << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MOUSE_MOVE)
		EVENT_CLASS_CATEGORY(EventCategory::MOUSE | EventCategory::INPUT)

	private:
		float m_MouseX, m_MouseY;
	};


	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float offsetX, float offsetY)
			: m_OffsetX(offsetX), m_OffsetY(offsetY)
		{
		}
		inline float GetOffsetX() const { return m_OffsetX; }
		inline float GetOffsetY() const { return m_OffsetY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: (" << m_OffsetX << ", " << m_OffsetY << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MOUSE_SCROLLED)
		EVENT_CLASS_CATEGORY(EventCategory::MOUSE | EventCategory::INPUT)

	private:
		float m_OffsetX, m_OffsetY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategory::MOUSE | EventCategory::MOUSE_BUTTON | EventCategory::INPUT)
	protected:
		MouseButtonEvent(int mouseCode)
			: m_Button(mouseCode)
		{
		}

		int m_Button;
	};

	class MousePressEvent : public MouseButtonEvent
	{
	public:
		MousePressEvent(int mouseCode, int repeatCount)
			: MouseButtonEvent(mouseCode), m_RepeatCount(repeatCount)
		{
		}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MousePressedEvent: " << m_Button << " (" << m_RepeatCount << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MOUSE_BUTTON_PRESSED)

	private:
		int m_RepeatCount;
	};

	class MouseReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseReleasedEvent(int keycode)
			: MouseButtonEvent(keycode) {
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MOUSE_BUTTON_RELEASED)
	};

}



