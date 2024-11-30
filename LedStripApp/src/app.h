#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <ranges>

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

	// Fetching settings
	std::wstring fetch_settings_path();
	void load_settings();
	void save_settings();

	// Updating
	bool create_new_controller(std::string name);
	bool update_controller(int index);
	bool delete_controller(int index);
	bool create_new_config(std::string name);
	bool update_controller_config(int index);
	bool delete_config(int index);

	// Getters
	LEDController* led_controller();
	std::vector<std::string> led_controller_names();
	std::vector<std::string> led_config_names();

private:
	Window m_window;

	friend class LEDController;
	std::vector<std::unique_ptr<LEDController>> m_led_controllers;
	int m_selected_controller;

	std::vector<std::unique_ptr<LEDConfiguration>> m_led_configs;
	std::map<std::string, int> m_selected_controller_configs;

    AppTab* m_current_tab = nullptr;

    friend class LightTab;
    LightTab m_light_tab = LightTab(this, "Light");

    AppTab* m_tabs[1] = { &m_light_tab };
};