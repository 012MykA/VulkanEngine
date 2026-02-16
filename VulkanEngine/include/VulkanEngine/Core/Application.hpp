#pragma once

#include "Window.hpp"

#include <memory>

namespace VE
{
    class Application
    {
    public:
        Application(const WindowConfig &config = WindowConfig());
        virtual ~Application() = default;

        void Run();

    private:
        std::unique_ptr<Window> m_Window;
    };

}
