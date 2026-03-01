#pragma once

#include "VulkanEngine/Core/Application.hpp"

#include <stdexcept>
#include <memory>
#include <iostream>

namespace VE
{
    int Main(int argc, char **argv)
    {
        try
        {
            std::unique_ptr<VE::Application> app(VE::CreateApplication());

            app->Run();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Application crashed: " << e.what() << std::endl;

#ifdef VE_DEBUG
            std::cin.get();
#endif
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

}

#if defined(VE_DIST) && defined(VE_PLATFORM_WINDOWS)

#include <Windows.h>

#pragma comment(linker, "/entry:wWinMainCRTStartup") // TODO: refactor

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    std::cout << "wWinMain()" << std::endl;
    return VE::Main(__argc, __argv);
}

#else

int main(int argc, char **argv)
{
    std::cout << "main()" << std::endl;
    return VE::Main(argc, argv);
}

#endif
