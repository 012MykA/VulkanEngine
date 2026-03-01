#include "Application.hpp"

#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Events/MouseEvent.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"

namespace VE
{
    void Application::Run()
    {
        WindowResizeEvent e(1280, 720);
        VE_TRACE(e.ToString());

        while (true)
        {
        }
    }

} // namespace VE
