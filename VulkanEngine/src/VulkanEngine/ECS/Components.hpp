#pragma once

#include "VulkanEngine/Renderer/Mesh.hpp"
#include "VulkanEngine/Renderer/MaterialPBR.hpp"

#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ve
{
    struct TagComponent
    {
        std::string tag;

        TagComponent(const std::string &tag) : tag(tag) {}
    };

    class TransformComponent
    {
    public:
        TransformComponent() = default;
        ~TransformComponent() = default;

    public:
        const glm::mat4 &GetTransform() const
        {
            if (m_IsDirty)
            {
                UpdateCache();
            }
            return m_CachedMatrix;
        }

    public:
        const glm::vec3 &GetPosition() const { return m_Position; }
        const glm::quat &GetRotation() const { return m_Rotation; }
        const glm::vec3 &GetScale() const { return m_Scale; }

        void SetPosition(const glm::vec3 &p)
        {
            m_Position = p;
            m_IsDirty = true;
        }

        void SetRotation(const glm::quat &r)
        {
            m_Rotation = r;
            m_IsDirty = true;
        }

        void SetScale(const glm::vec3 &s)
        {
            m_Scale = s;
            m_IsDirty = true;
        }

    private:
        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_Scale = glm::vec3(1.0f);

        mutable glm::mat4 m_CachedMatrix = glm::mat4(1.0f);
        mutable bool m_IsDirty = true;

        void UpdateCache() const
        {
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_Position);
            glm::mat4 rotation = glm::mat4_cast(m_Rotation);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_Scale);

            m_CachedMatrix = translation * rotation * scale;

            m_IsDirty = false;
        }
    };

    struct MeshComponent
    {
        std::shared_ptr<Mesh> mesh;

        MeshComponent(std::shared_ptr<Mesh> mesh) : mesh(std::move(mesh)) {}
    };

    struct MaterialPBRComponent
    {
        std::shared_ptr<MaterialPBR> material;

        MaterialPBRComponent(std::shared_ptr<MaterialPBR> material) : material(std::move(material)) {}
    };

} // namespace ve
