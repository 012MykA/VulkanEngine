#pragma once

#include "Scene.hpp"

#include <entt/entt.hpp>

#include <cassert>

namespace ve
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene *scene);
        ~Entity() = default;

        Entity(const Entity &) = default;

    public:
        template <typename T, typename... Args>
        T &AddComponent(Args &&...args)
        {
            assert(!HasComponent<T>() && "Entity already has component!");
            T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

            return component;
        }

        template <typename T, typename... Args>
        T &AddOrReplaceComponent(Args &&...args)
        {
            T &component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            return component;
        }

        template <typename T>
        T &GetComponent() const
        {
            assert(HasComponent<T>() && "Entity does not have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template <typename T>
        bool HasComponent() const
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        template <typename T>
        void RemoveComponent()
        {
            assert(HasComponent<T>() && "Entity does not have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

    public:
        operator entt::entity() const { return m_EntityHandle; }

    private:
        entt::entity m_EntityHandle = entt::null;
        Scene *m_Scene = nullptr;
    };

} // namespace ve
