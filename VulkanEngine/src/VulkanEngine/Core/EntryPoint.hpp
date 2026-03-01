#pragma once

#include "VulkanEngine/Core/Application.hpp"

#include <stdexcept>
#include <memory>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        std::unique_ptr<VE::Application> app(VE::CreateApplication());
        
        app->Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}