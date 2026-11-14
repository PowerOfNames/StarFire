#pragma once
#include "StarFire/Core/Identifier.h"

#include <entt/entt.hpp>

#include <string>
#include <string_view>

namespace StarFire {

	class Entity;
	class Scene
	{
		friend class Entity;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(UniversalID id, std::string_view name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(double deltaTime);

	private:
		entt::registry m_Registry;
	};
}
