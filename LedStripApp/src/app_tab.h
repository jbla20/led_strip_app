#pragma once

#include <string>

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot_internal.h"

class App;

class AppTab
{
public:
	explicit AppTab(App* app, std::string name) : m_app(app), m_name(name) { }
	~AppTab() = default;

    virtual void render() = 0;
    // Called when tab is opened
    virtual void on_open() {}
    std::string name() { return m_name; }
    bool m_reset_tab = true;
    bool m_first_frame = false; // Workaround for https://github.com/ocornut/imgui/issues/5005

protected:
    App* m_app;
    std::string m_name;
};