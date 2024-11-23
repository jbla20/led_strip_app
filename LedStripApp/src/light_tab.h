#pragma once

#include <iostream>

#include "app_tab.h"

class App;

class LightTab : public AppTab
{
public:
    explicit LightTab(App* app, std::string name);

    void render() override;
    void on_open() override;
};