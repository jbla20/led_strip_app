#pragma once

#include <chrono>
#include <iostream>

class TimerConfiguration
{
public:
    explicit TimerConfiguration(std::string name, float start, float end, int repeat, bool inverse)
        : name(name), start(start), end(end), repeat(repeat), inverse(inverse) {}
	~TimerConfiguration() = default;

    inline float get_progress_percentage() { return progress / static_cast<float>(repeat); }
    inline bool is_done() { return progress > static_cast<float>(repeat) || start >= end; }

protected:
    inline void update_progress(float time)
    { 
        try
        {
            if (end == 0)
            {
                throw std::runtime_error("End time must be greater than zero");
            }
            progress = time / end; 
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

public:
    std::string name;
    float start;
    float end;
    int repeat;
    bool inverse;

protected:
    float progress = 0.0f; // Range: 0-repeat

    friend class Timer;
};