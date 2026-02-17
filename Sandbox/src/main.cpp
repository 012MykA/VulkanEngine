#include "VulkanEngine.hpp"

#include <stdexcept>
#include <iostream>

class VulkanApp : public VE::Application
{
public:
    VulkanApp() : Application("Vulkan App")
    {
    }
};

int main()
{
    try
    {
        VulkanApp app;
        app.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
