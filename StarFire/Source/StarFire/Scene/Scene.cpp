#include "sfpch.h"
#include "StarFire/Scene/Scene.h"
#include "StarFire/Scene/Entity.h"
#include "StarFire/Scene/Components.h"

#include "StarFire/Core/Core.h"

namespace StarFire {

	Scene::Scene()
	{
		PROFILE_FUNCTION;

		m_Registry = entt::registry();
	}
	
	Scene::~Scene()
	{
		PROFILE_FUNCTION;

		m_Registry.clear();
	}

	void Scene::OnUpdate(double deltaTime)
	{
		PROFILE_FUNCTION;

	}

	Entity Scene::CreateEntity(UniversalID id, std::string_view name /*= std::string()*/)
	{
		PROFILE_FUNCTION;
	
		Entity entity(m_Registry.create(), this);
		entity.AddComponent<IdComponent>(id);
		entity.AddComponent<TagComponent>(std::string(name));

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		PROFILE_FUNCTION;
		if (!entity)
			return;
		m_Registry.destroy(entity);
	}
}