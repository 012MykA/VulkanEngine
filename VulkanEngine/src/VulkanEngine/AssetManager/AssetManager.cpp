#include "AssetManager.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    void AssetManager::Init(const std::filesystem::path &projectRoot, const std::filesystem::path &engineRoot)
    {
        s_ProjectRoot = std::filesystem::absolute(projectRoot);

        if (engineRoot.empty())
            s_EngineRoot = std::filesystem::current_path();
        else
            s_EngineRoot = std::filesystem::absolute(engineRoot);

        VE_CORE_INFO("AssetManager Initialized:");
        VE_CORE_INFO("  Project Root: {0}", s_ProjectRoot.string());
        VE_CORE_INFO("  Engine Root:  {0}", s_EngineRoot.string());
    }

    void AssetManager::Shutdown()
    {
        std::lock_guard<std::mutex> lock(s_CacheMutex);
        s_AssetCache.clear();
    }

    std::filesystem::path AssetManager::ResolvePath(const std::string &virtualPath)
    {
        if (virtualPath.starts_with("@engine/"))
        {
            return s_EngineRoot / virtualPath.substr(8);
        }

        if (virtualPath.starts_with("@project/"))
        {
            return s_ProjectRoot / virtualPath.substr(9);
        }

        return s_ProjectRoot / virtualPath;
    }

} // namespace ve
