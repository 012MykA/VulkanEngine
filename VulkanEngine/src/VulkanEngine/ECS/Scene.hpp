#pragma once

#include <entt/entt.hpp>

namespace ve
{
    class Entity;
    
    class Scene
    {
        friend class Entity;
        friend class CullingSystem;

    public:
        Scene() = default;
        ~Scene() = default;

    public:
        Entity CreateEntity(const std::string &tag = "Unnamed");
        void DestroyEntity(Entity entity);

    private:
        entt::registry m_Registry;
    };

} // namespace ve
