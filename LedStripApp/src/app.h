#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <ranges>

#include "window.h"
#include "led_controller.h"
#include "led_configuration.h"
#include "timer.h"
#include "timer_configuration.h"
#include "app_tab.h"
#include "light_tab.h"
#include "log_tab.h"

class App
{
public:
	App();
	~App();

	bool init();
	void run();

private:
	void render();

	// Fetching settings
	std::wstring fetch_settings_path();
	void load_settings();
	void save_settings();

	// Updating
	bool create_new_controller(std::string name);
	bool update_controller(int index);
	bool delete_selected_controller();
	bool rename_selected_controller(std::string new_name);
	bool create_new_led_config(std::string name);
	bool update_controller_led_config(int index);
	bool delete_selected_led_config();
	bool rename_selected_led_config(std::string new_name);
	bool create_new_timer_config(std::string name);
	bool update_controller_timer_config(int index);
	bool delete_selected_timer_config();
	bool rename_selected_timer_config(std::string new_name);

	// Getters
	LEDController* led_controller();
	std::vector<std::string> led_controller_names();
	std::vector<std::string> led_controller_aliases();
	std::vector<std::string> led_config_names();
	std::vector<std::string> timer_config_names();

private:
	Window m_window;

	friend class LEDController;
	std::vector<std::unique_ptr<LEDController>> m_led_controllers;
	int m_selected_controller;

	std::vector<std::unique_ptr<LEDConfiguration>> m_led_configs;
	std::map<std::string, int> m_selected_led_configs;

	friend class Timer;
	Timer m_timer;
	std::vector<std::unique_ptr<TimerConfiguration>> m_timer_configs;
	std::map<std::string, int> m_selected_timer_configs;

    AppTab* m_current_tab = nullptr;

    friend class LightTab;
	friend class LogTab;
    LightTab m_light_tab = LightTab(this, "Light");
	LogTab m_log_tab = LogTab(this, "Log");

    AppTab* m_tabs[2] = { &m_light_tab, &m_log_tab };
};