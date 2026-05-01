#pragma once

// --- Application ----------------------------------
#include "VulkanEngine/Core/Application.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Layer.hpp"
#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"
#include "VulkanEngine/Events/MouseEvent.hpp"
// --------------------------------------------------

// --- Assets ---------------------------------------
#include "VulkanEngine/Assets/GLTFScene.hpp"
#include "VulkanEngine/Assets/GLTFLoader.hpp"

#include "VulkanEngine/Assets/MaterialPBR.hpp"
#include "VulkanEngine/Assets/MaterialLoader.hpp"

#include "VulkanEngine/Assets/Vertex.hpp"
#include "VulkanEngine/Assets/Mesh.hpp"
#include "VulkanEngine/Assets/MeshLoader.hpp"

#include "VulkanEngine/Assets/Texture.hpp"
#include "VulkanEngine/Assets/TextureLoader.hpp"
// --------------------------------------------------

// --- Renderer -------------------------------------
#include "VulkanEngine/Renderer/RenderSettings.hpp"
#include "VulkanEngine/Renderer/RendererPBR.hpp"
#include "VulkanEngine/Renderer/SceneRenderer.hpp"
#include "VulkanEngine/Renderer/AABB.hpp"
#include "VulkanEngine/Renderer/Camera/Camera.hpp"
#include "VulkanEngine/Renderer/Camera/FPSCamera.hpp"
// --------------------------------------------------

// --- ECS ------------------------------------------
#include "VulkanEngine/ECS/Entity.hpp"
#include "VulkanEngine/ECS/Components.hpp"
#include "VulkanEngine/ECS/Scene.hpp"
#include "VulkanEngine/ECS/CullingSystem.hpp"
// --------------------------------------------------

// --- Input ----------------------------------------
#include "VulkanEngine/Core/KeyCodes.hpp"
#include "VulkanEngine/Core/MouseCodes.hpp"
// --------------------------------------------------

// --- Utils ----------------------------------------
#include "VulkanEngine/Core/Timer.hpp"
// --------------------------------------------------

// --- Layers ---------------------------------------
#include "VulkanEngine/Layers/FPSLayer.hpp"
// --------------------------------------------------
