#pragma once

#include "VulkanEngine/Core/Base.hpp"

namespace VE
{
    class Application
    {        
    public:
        Application() = default;
        virtual ~Application() {}

    public:
        void Run();
    
    };

    // To be defined in a CLIENT
    Application *CreateApplication();

} // namespace VE
