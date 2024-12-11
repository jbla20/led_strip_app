#include <iostream>
#include <ranges>

#include "timer.h"
#include "app.h"

Timer::Timer(App* app) : m_app(app), m_start_time(clock::now()), m_delta_time_s(0.0f), m_paused(true)
{
}

bool Timer::update()
{
    if (m_paused) {
        return false;
    }

    m_delta_time_s = std::chrono::duration<float>(clock::now() - m_start_time).count();

    auto in_active_range = [this](const TimerConfiguration* timer_config) -> bool
    {
        const float cycle_duration = timer_config->end;
        
        return (std::fmod(m_delta_time_s, cycle_duration) > timer_config->start) &&
               (std::fmod(m_delta_time_s, cycle_duration) < timer_config->end);
    };

    for (size_t i = 1; i < m_app->m_led_controllers.size(); i++)
    {
        LEDController* controller = m_app->m_led_controllers[i].get();

        if (controller->timer_config()->is_done())
        {
            continue;
        }
        controller->timer_config()->update_progress(m_delta_time_s);

        if (in_active_range(controller->timer_config()))
        {
            if (!controller->is_device_on() != controller->timer_config()->inverse)
            {
                m_app->m_led_controllers[i]->toggle_device();
            }
        }
        else
        {
            if (controller->is_device_on() != controller->timer_config()->inverse)
            {
                m_app->m_led_controllers[i]->toggle_device();
            }
        }
    }

    return true;
}

void Timer::pause(bool pause) 
{
    if (pause == m_paused)
    {
        return;
    }

    if (pause == false)
    {
        m_start_time = clock::now() - std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(m_delta_time_s));
    }

    m_paused = pause;
}

void Timer::reset()
{
    m_delta_time_s = 0.0f;
    for (size_t i = 1; i < m_app->m_timer_configs.size(); i++)
    {
        m_app->m_timer_configs[i]->update_progress(0.0f);
    }

    pause(true);
}
