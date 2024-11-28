#include <vector>
#include <ranges>
#include <algorithm>

#include "light_tab.h"
#include "app.h"

LightTab::LightTab(App* app, std::string name) : AppTab(app, name) 
{ 
}

void LightTab::on_open()
{
}

void LightTab::render()
{
    ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
    float title_bar_height = ImGui::GetFrameHeight();
    float menu_bar_height = ImGui::GetFrameHeightWithSpacing();
    ImVec2 window_size = ImGui::GetMainViewport()->WorkSize;
    window_size.y -= (title_bar_height + menu_bar_height);
    ImGui::DockSpace(dockspace_id, window_size);
    if (m_reset_tab)
    {
        m_reset_tab = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, nullptr, &dockspace_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.4f, nullptr, &dockspace_id);
        ImGuiID dock_id_center = dockspace_id;  // The remaining space in the center

        ImGui::DockBuilderDockWindow("Bluetooth Connect", dock_id_left);
        ImGui::DockBuilderDockWindow("Light Configs", dock_id_center);
        ImGui::DockBuilderDockWindow("Light Settings", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    // Creating ui for different dock windows
    if (ImGui::Begin("Bluetooth Connect")) {
        // Create new known device
        if (ImGui::InputText("##New device name", m_new_controller_name, sizeof(m_new_controller_name), ImGuiInputTextFlags_CharsNoBlank));
        if (ImGui::Button("Create"))
        {
            auto name_exists = [&](char in_str[]) {
                return std::ranges::any_of(m_app->led_controller_names(), [in_str](const std::string& name) { return std::string(in_str) == name; });
            };
            if (m_new_controller_name[0] != '\0' && !name_exists(m_new_controller_name))
            {
                if (m_app->create_new_controller(std::string(m_new_controller_name)))
                {
                    std::cout << "Created new controller." << std::endl;
                    m_app->update_controller(m_app->m_led_controllers.size() - 1);
                }
            }
        }

        // List all known devices
        std::vector<const char*> controller_items;
        controller_items.reserve(controller_items.size());
        std::vector<std::string> controller_names = m_app->led_controller_names();
        for (const auto& item : controller_names)
        {
            controller_items.push_back(item.c_str());
        }
        ImGui::Text("Known Devices");
        if (ImGui::ListBox(" ", &m_selected_controller, controller_items.data(), controller_items.size(), 10))
        {
            m_app->update_controller(m_selected_controller + 1);
        }

        // Connect
        // TODO: Implement deleting of controllers
        ImGui::Text(m_app->led_controller()->connection_status_str().c_str());
        if (!m_app->led_controller()->is_scanning())
        {
            if (!m_app->led_controller()->is_connected())
            {
                if (ImGui::Button("Connect"))
                {
                    m_app->led_controller()->scan_and_connect();
                }
            }
        }
    }
    ImGui::End(); // Bluetooth Connect

    if (ImGui::Begin("Light Configs")) 
    {
        std::vector<const char*> config_items;
        config_items.reserve(config_items.size());
        std::vector<std::string> config_names = m_app->led_config_names();
        for (const auto& item : config_names)
        {
            config_items.push_back(item.c_str());
        }
        ImGui::Text("Available configs");
        if (ImGui::ListBox(" ", &m_selected_config, config_items.data(), config_items.size(), 10))
        {
            m_app->update_controller_config(m_selected_config + 1);
        }

        ImGui::Text("New config name");
        if (ImGui::InputText("##New config name", m_new_config_name, sizeof(m_new_config_name), ImGuiInputTextFlags_CharsNoBlank))
        {
        }
        ImGui::SameLine();
        if (ImGui::Button("Create"))
        {
            auto name_exists = [&](char in_str[]) {
                return std::ranges::any_of(m_app->led_config_names(), [in_str](const std::string& name) { return std::string(in_str) == name; } );
            };
            if (m_new_config_name[0] != '\0' && !name_exists(m_new_config_name))
            {
                if (m_app->create_new_config(std::string(m_new_config_name)))
                {
                    std::cout << "Created new config." << std::endl;
                    m_selected_config = -1;
                }
            }
        }
    }
    ImGui::End(); // Light Configs

    if (ImGui::Begin("Light Settings"))
    {
        ImGui::Text("Device Status");
        if (ImGui::Button(m_app->led_controller()->is_device_on() ? "On" : "Off"))
        {
            m_app->led_controller()->toggle_device();
        }
        ImGui::Text("Color Selection");
        if (ImGui::ColorEdit3("Color", m_app->led_controller()->led_config()->color.data()))
        {
            m_app->led_controller()->update_rgb();
        }
        if (ImGui::SliderFloat("Brightness", &m_app->led_controller()->led_config()->brightness, 0, 1))
        {
            m_app->led_controller()->update_rgb();
        }

        ImGui::Text("Mode Selection");
        if (ImGui::Combo("Mode", &m_app->led_controller()->led_config()->mode.index, m_app->led_controller()->led_config()->mode.mode_strings, IM_ARRAYSIZE(m_app->led_controller()->led_config()->mode.mode_strings)))
        {
            m_app->led_controller()->update_mode();
        }
        if (ImGui::SliderFloat("Speed", &m_app->led_controller()->led_config()->mode.speed, 0, 1))
        {
            m_app->led_controller()->update_mode();
        }
    }
    ImGui::End(); // Light Settings
}