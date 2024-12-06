#pragma once

#include <chrono>
#include <vector>

#include "timer_configuration.h"

class App;

class Timer
{
public:
    explicit Timer(App* app);
	~Timer() = default;
	
	bool update();
	void pause(bool val);
	void reset();

	inline float get_relative_time() { return m_delta_time_s; }
	inline bool is_paused() { return m_paused; }
	inline bool is_active() { return m_delta_time_s > 0.001f; }

protected:
	using clock = std::chrono::high_resolution_clock;

	std::chrono::time_point<clock> m_start_time;
	float m_delta_time_s;
	bool m_paused;
	App* m_app;
};