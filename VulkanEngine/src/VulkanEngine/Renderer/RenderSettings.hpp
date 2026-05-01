#pragma once

#include <cstdint>

namespace ve
{
    enum class AnisotropicLevel : uint8_t
    {
        Off = 0,
        X2 = 2,
        X4 = 4,
        X8 = 8,
        X16 = 16,
    };

    enum class MSAASamples : uint8_t
    {
        Off = 1,
        X2 = 2,
        X4 = 4,
        X8 = 8,
        X16 = 16,
        X32 = 32,
        X64 = 64,
    };

    enum class ShadowQuality : uint16_t
    {
        Off = 0,
        Low = 512,
        Medium = 1024,
        High = 2048,
        Ultra = 4096,
    };

    struct RenderSettings
    {
        AnisotropicLevel anisotropy = AnisotropicLevel::Off;

        MSAASamples msaaSamples = MSAASamples::Off;

        ShadowQuality shadowQuality = ShadowQuality::Off;

        float iblIntensity = 1.0f;
    };

} // namespace ve
