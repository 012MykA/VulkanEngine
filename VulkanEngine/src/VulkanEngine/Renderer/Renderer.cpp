#include "Renderer.hpp"

namespace ve
{
    Renderer::Renderer()
    {
        m_Instance = std::make_unique<VulkanInstance>(InstanceDesc{
#ifdef VE_DEBUG
            .enableValidation = true,
            .debugMessenger{
                .enableDebugMessenger = true,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            },
#endif
        });
    }

} // namespace ve
