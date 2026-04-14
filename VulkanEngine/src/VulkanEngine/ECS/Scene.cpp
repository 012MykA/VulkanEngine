#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"

namespace ve
{
    Entity Scene::CreateEntity(const std::string &tag)
    {
        Entity entity(m_Registry.create(), this);
        entity.AddComponent<TagComponent>(tag);

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

} // namespace ve
