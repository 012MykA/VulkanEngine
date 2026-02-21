#pragma once

#include <chrono>

class Timer
{
private:
    using Clock = std::chrono::high_resolution_clock;

public:
    Timer()
    {
        Reset();
    }

    void Reset()
    {
        m_Start = Clock::now();
    }

    double ElapsedSeconds() const
    {
        return std::chrono::duration<double>(Clock::now() - m_Start).count();
    }

    double ElapsedMilliseconds() const
    {
        return std::chrono::duration<double, std::milli>(Clock::now() - m_Start).count();
    }

private:
    std::chrono::time_point<Clock> m_Start;
};
