#pragma once
#include "StarFire/Core/BitField.h"


#include <string>
#include <functional>


namespace StarFire {

	enum class EventType
	{
		NONE = 0,
		WINDOW_CLOSE, 
		WINDOW_MINIMIZE,
		WINDOW_RESIZE,
		WINDOW_FOCUS,
		WINDOW_LOST_FOCUS,
		WINDOW_MOVE,

		FRAMEBUFFER_RESIZE,

		APP_TICK,
		APP_UPDATE,
		APP_RENDER,

		KEY_PRESSED,
		KEY_RELEASED,
		KEY_TYPED,

		MOUSE_BUTTON_PRESSED,
		MOUSE_BUTTON_RELEASED,
		MOUSE_MOVE,
		MOUSE_SCROLLED
	};

	enum class EventCategory : BitField8
	{
		NONE			= 0,
		APPLICATION		= BIT(0),
		INPUT			= BIT(1),
		KEYBOARD		= BIT(2),
		MOUSE			= BIT(3),
		MOUSE_BUTTON	= BIT(4)
	};
	SF_ENABLE_BIT_OPS(EventCategory);

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual EventCategory GetCategoryFlags() const override { return static_cast<EventCategory>(category); }

	class Event
	{
		friend class EventDispatcher;
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual EventCategory GetCategoryFlags() const = 0;
		//Defines whether or not the event type history is of value (key input -> yes, so coalescent is false, window resize/mouse move -> not, so coalescent is true)
		virtual bool IsCoalescent() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) { return FieldHasFlag(GetCategoryFlags(), category); }

	public:
		bool Handled = false;
	};


	class EventDispatcher
	{
		template<typename T>
		using EventFN = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& e)
			: m_Event(e)
		{
		}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() != T::GetStaticType())
				return false;
			
			m_Event.Handled = func(static_cast<T&>(m_Event));
			return true;			
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.ToString(); }
}
