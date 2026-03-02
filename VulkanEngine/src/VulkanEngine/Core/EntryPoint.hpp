#pragma once

#include "VulkanEngine/Core/Application.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stdexcept>
#include <memory>
#include <iostream>

namespace ve
{
    int Main(int argc, char **argv)
    {
        Log::Init();
        
        try
        {
            std::unique_ptr<ve::Application> app(ve::CreateApplication());

            app->Run();
        }
        catch (const std::exception &e)
        {
            VE_CORE_CRITICAL("{0}", e.what());

#ifdef VE_DEBUG
            std::cin.get();
#endif
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

}

// Entry Point
#if defined(VE_DIST) && defined(VE_PLATFORM_WINDOWS)

#include <Windows.h>

#pragma comment(linker, "/entry:wWinMainCRTStartup") // TODO: refactor

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return ve::Main(__argc, __argv);
}

#else

int main(int argc, char **argv)
{
    return ve::Main(argc, argv);
}

#endif // End Entry Point
