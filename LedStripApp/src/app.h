#pragma once

#include "window.h"
#include "led_controller.h"
#include "app_tab.h"
#include "light_tab.h"

class App
{
public:
	App();
	~App();

	bool init();
	void run();

private:
	void render();

	std::wstring fetch_settings_path();
	void load_settings();
	void save_settings();

private:
	Window m_window;
	LEDController m_led_controller;

    AppTab* m_current_tab = nullptr;

    friend class LightTab;
    LightTab m_light_tab = LightTab(this, "Light");

    AppTab* m_tabs[1] = { &m_light_tab };
};