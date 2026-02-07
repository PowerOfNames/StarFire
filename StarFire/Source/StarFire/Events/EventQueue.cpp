#include "sfpch.h"
#include "StarFire/Core/Assert.h"
#include "StarFire/Events/ApplicationEvent.h"
#include "StarFire/Events/EventQueue.h"
#include "StarFire/Events/MouseEvent.h"

#include <concurrentqueue.h>

namespace StarFire {
	
	struct WindowResizeData
	{
		uint32_t Width;
		uint32_t Height;
	};
	
	struct FramebufferResizeData
	{
		uint32_t Width;
		uint32_t Height;
	};

	struct MouseMoveData
	{
		float X;
		float Y;
	};
	
	struct MouseScrollData
	{
		float OffsetX;
		float OffsetY;
	};

	class EventQueueImpl
	{
	public:
		moodycamel::ConcurrentQueue<Scope<Event>> m_Queue;

		//One mutex should be enough, if not, implement for each
		std::mutex m_CoalescedMutex;
		std::optional<WindowResizeData> m_WindowResizeData;
		std::optional<FramebufferResizeData> m_FramebufferResizeData;
		std::optional<MouseMoveData> m_MouseMoveData;
		std::optional<MouseScrollData> m_MouseScrollData;
	};


	EventQueue::EventQueue(size_t initialSize /*= 100*/)
		: m_Impl(CreateScope<EventQueueImpl>())
	{
		m_Impl->m_Queue = moodycamel::ConcurrentQueue<Scope<Event>>(initialSize);
		m_Initialized = true;
	}
	EventQueue::~EventQueue() = default;

	void EventQueue::Destruct()
	{
		if (!m_Initialized)
			return;
		m_Impl->m_Queue.~ConcurrentQueue();
	}

	void EventQueue::Push(Scope<Event> e)
	{
		SF_CORE_ASSERT(m_Initialized, "EventQueue needs to be initialized first!");
		
		if (e->IsCoalescent())
			HandleCoalescent(std::move(e));
		else
			m_Impl->m_Queue.enqueue(std::move(e));
	}

	void EventQueue::GatherCoalescing()
	{
		std::lock_guard<std::mutex> lock(m_Impl->m_CoalescedMutex);
		if (m_Impl->m_WindowResizeData.has_value())
		{
			m_Impl->m_Queue.enqueue(CreateScope<WindowResizeEvent>(
				m_Impl->m_WindowResizeData->Width, 
				m_Impl->m_WindowResizeData->Height));
			m_Impl->m_WindowResizeData.reset();
		}
		if (m_Impl->m_FramebufferResizeData.has_value())
		{
			m_Impl->m_Queue.enqueue(CreateScope<FramebufferResizeEvent>(
				m_Impl->m_FramebufferResizeData->Width, 
				m_Impl->m_FramebufferResizeData->Height));
			m_Impl->m_FramebufferResizeData.reset();
		}
		if (m_Impl->m_MouseMoveData.has_value())
		{
			m_Impl->m_Queue.enqueue(CreateScope<MouseMoveEvent>(
				m_Impl->m_MouseMoveData->X, 
				m_Impl->m_MouseMoveData->Y));
			m_Impl->m_MouseMoveData.reset();
		}
		if (m_Impl->m_MouseScrollData.has_value())
		{
			m_Impl->m_Queue.enqueue(CreateScope<MouseScrolledEvent>(
				m_Impl->m_MouseScrollData->OffsetX, 
				m_Impl->m_MouseScrollData->OffsetY));
			m_Impl->m_MouseScrollData.reset();
		}	
	}

	bool EventQueue::Pop(Scope<Event>& e)
	{
		SF_CORE_ASSERT(m_Initialized, "EventQueue needs to be initialized first!");
		return m_Impl->m_Queue.try_dequeue(e);
	}

	void EventQueue::HandleCoalescent(Scope<Event> e)
	{
		std::lock_guard<std::mutex> lock(m_Impl->m_CoalescedMutex);
		switch (e->GetEventType())
		{
		case EventType::WINDOW_RESIZE:
		{
			auto& eventRef = static_cast<WindowResizeEvent&>(*e);
			if (m_Impl->m_WindowResizeData.has_value())
				m_Impl->m_WindowResizeData.emplace(eventRef.GetWidth(), eventRef.GetHeight());
			else
				m_Impl->m_WindowResizeData = { eventRef.GetWidth(), eventRef.GetHeight() };

			break;
		}
		case EventType::FRAMEBUFFER_RESIZE:
		{
			auto& eventRef = static_cast<FramebufferResizeEvent&>(*e);
			if (m_Impl->m_FramebufferResizeData.has_value())
				m_Impl->m_FramebufferResizeData.emplace(eventRef.GetWidth(), eventRef.GetHeight());
			else
				m_Impl->m_FramebufferResizeData = { eventRef.GetWidth(), eventRef.GetHeight() };

			break;
		}
		case EventType::MOUSE_MOVE:
		{
			auto& eventRef = static_cast<MouseMoveEvent&>(*e);
			if (m_Impl->m_MouseMoveData.has_value())
				m_Impl->m_MouseMoveData.emplace(eventRef.GetMouseX(), eventRef.GetMouseY());
			else
				m_Impl->m_MouseMoveData = { eventRef.GetMouseX(), eventRef.GetMouseY() };

			break;
		}
		case EventType::MOUSE_SCROLLED:
		{
			auto eventRef = static_cast<MouseScrolledEvent&>(*e);
			if (m_Impl->m_MouseScrollData.has_value())
			{
				m_Impl->m_MouseScrollData->OffsetX += eventRef.GetOffsetX();
				m_Impl->m_MouseScrollData->OffsetY += eventRef.GetOffsetY();
			}
			else
			{
				m_Impl->m_MouseScrollData = { eventRef.GetOffsetX(), eventRef.GetOffsetY() };
			}
			break;
		}
		default:
		{
			SF_CORE_WARN("Unknown event type. Event could not be coalesced!");
			return;
		}
		}
	}
}
