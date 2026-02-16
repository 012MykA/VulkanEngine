#include "VulkanEngine/VulkanEngine.hpp"

#include <stdexcept>
#include <iostream>

class HelloTriangleApplication : public VE::Application
{
public:
    HelloTriangleApplication() = default;
};

int main()
{
    try
    {        
        HelloTriangleApplication app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
