#pragma once
#include "StarFire/Core/Core.h"
#include "StarFire/Scene/Scene.h"


#include <entt/entt.hpp>

namespace StarFire {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);

		template<typename TComponent, typename ... TArgs>
		TComponent& AddComponent(TArgs&&... args)
		{			
			PROFILE_FUNCTION;

			return m_Scene->m_Registry.emplace<TComponent>(m_EntityHandle, std::forward<TArgs>(args)...);
		}

		template<typename TComponent>
		TComponent& GetComponent()
		{
			STARFIRE_ASSERT(HasComponent<TComponent>(), "Entity does not have component!");

			return m_Scene->m_Registry.get<TComponent>(m_EntityHandle);
		}

		template<typename TComponent>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<TComponent>(m_EntityHandle);
		}

		template<typename ... TComponent>
		bool HasComponents()
		{
			return m_Scene->m_Registry.all_of<TComponent...>(m_EntityHandle);
		}

		template<typename TComponent>
		void RemoveComponent()
		{
			PROFILE_FUNCTION;

			m_Scene->m_Registry.remove<TComponent>(m_EntityHandle);
		}

		explicit operator bool() const { return m_EntityHandle != entt::null && m_Scene && m_Scene->m_Registry.valid(m_EntityHandle); }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		constexpr operator entt::entity() const { return m_EntityHandle; }
		constexpr explicit operator uint32_t() const { return (uint32_t)m_EntityHandle; }

	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
	};

}
