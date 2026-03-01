#include "Application.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stdexcept>

namespace VE
{
    void Application::Run()
    {
        VE_CORE_TRACE("Trace test: {0}", "Text");
        VE_CORE_INFO("Info test: {0}, {1}", true, 43);
        VE_CORE_WARN("");
        VE_CORE_ERROR("Error test: {}, {}", 0, "OKAY");

        while (true)
        {
            throw std::runtime_error("exception test!");
        }
    }

} // namespace VE
