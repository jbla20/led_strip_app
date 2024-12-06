#pragma once

#include <chrono>
#include <iostream>

class TimerConfiguration
{
public:
    explicit TimerConfiguration(std::string name, bool enabled, float delay, float duration, int repeat)
        : name(name), enabled(enabled), delay(delay), duration(duration), repeat(repeat) {}
	~TimerConfiguration() = default;

    inline float get_progress_percentage() { return progress / static_cast<float>(repeat); }
    inline bool is_done() { return progress > static_cast<float>(repeat); }

protected:
    inline void update_progress(float time)
    { 
        try
        {
            if (duration == 0)
            {
                throw std::runtime_error("Duration must be greater than zero");
            }
            progress = time / (delay + duration); 
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

public:
    std::string name;
    bool enabled;
    float delay;
    float duration;
    int repeat;

protected:
    float progress = 0.0f; // Range: 0-repeat

    friend class Timer;
};