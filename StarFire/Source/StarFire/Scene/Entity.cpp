#include "sfpch.h"
#include "StarFire/Scene/Entity.h"


namespace StarFire {	

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene), m_EntityHandle(handle)
	{
		STARFIRE_ASSERT(m_Scene != nullptr, "Scene pointer is null!");
	}

}