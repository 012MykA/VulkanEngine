#pragma once

#include "VulkanEngine/Core/Log.hpp"
#include "Asset.hpp"

#include <memory>
#include <map>
#include <mutex>

namespace ve
{
    class AssetManager
    {
    public:
        static void Init(const std::filesystem::path &projectRoot, const std::filesystem::path &engineRoot = "");
        static void Shutdown();

        template <typename T, typename... Args>
        static std::shared_ptr<T> GetAsset(const std::string &virtualPath, Args &&...args)
        {
            std::lock_guard<std::mutex> lock(s_CacheMutex);

            if (s_AssetCache.contains(virtualPath))
            {
                return std::static_pointer_cast<T>(s_AssetCache[virtualPath]);
            }

            std::filesystem::path absolutePath = ResolvePath(virtualPath);

            try
            {
                auto asset = std::make_shared<T>(absolutePath, std::forward<Args>(args)...);
                asset->FilePath = virtualPath;
                s_AssetCache[virtualPath] = asset;
                return asset;
            }
            catch (const std::exception &e)
            {
                VE_CORE_ERROR("AssetManager: Failed to load '{0}'. Error: {1}", virtualPath, e.what());
                return nullptr;
            }
        }

        static std::filesystem::path ResolvePath(const std::string &virtualPath);

    private:
        inline static std::filesystem::path s_ProjectRoot;
        inline static std::filesystem::path s_EngineRoot;
        inline static std::map<std::string, std::shared_ptr<Asset>> s_AssetCache;
        inline static std::mutex s_CacheMutex;
    };

} // namespace ve
