#pragma once

#include <string>

#include "app_tab.h"

class App;

class LightTab : public AppTab
{
public:
    explicit LightTab(App* app, std::string name);

    void render() override;
    void on_open() override;

private:
    // Data used across windows in this tab
    int m_selected_controller = 0;
    char m_new_controller_name[100] = "\0";
    char m_rename_controller_name[100] = "\0";

    int m_selected_led_config = 0;
    char m_new_led_config_name[100] = "\0";
    char m_rename_led_config_name[100] = "\0";

    int m_selected_timer_config = 0;
    char m_new_timer_config_name[100] = "\0";
    char m_rename_timer_config_name[100] = "\0";
};