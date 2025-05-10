#include "sfpch.h"
#include "StarFire/Events/EventQueue.h"
#include "StarFire/Core/Assert.h"

#include <concurrentqueue.h>


namespace StarFire {

	class EventQueueImpl
	{
	public:
		moodycamel::ConcurrentQueue<Scope<Event>> m_Queue;
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

	}

	void EventQueue::Push(Scope<Event> e)
	{
		SF_CORE_ASSERT(m_Initialized, "EventQueue needs to be initialized first!");

		m_Impl->m_Queue.enqueue(std::move(e));
	}


	bool EventQueue::Pop(Scope<Event>& e)
	{
		SF_CORE_ASSERT(m_Initialized, "EventQueue needs to be initialized first!");
		return m_Impl->m_Queue.try_dequeue(e);
	}

}
