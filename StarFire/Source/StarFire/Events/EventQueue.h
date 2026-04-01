#pragma once
#include "StarFire/Events/Event.h"
#include "StarFire/Core/Core.h"
#include "StarFire/Core/Assert.h"


namespace StarFire {

	class EventQueueImpl;
	class EventQueue
	{
	public:
		EventQueue(size_t initialSize = 100);
		~EventQueue();

		//Only call when all using threads have been closed
		void Destruct();

		void Push(Scope<Event> e);
		void GatherCoalescing();
		bool Pop(Scope<Event>& e);

		

	private:
		void HandleCoalescent(Scope<Event> e);

	private:
		Scope<EventQueueImpl> m_Impl;
		bool m_Initialized = false;
	};
}
