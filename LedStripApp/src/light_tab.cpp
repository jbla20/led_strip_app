#include "light_tab.h"
#include "app.h"

LightTab::LightTab(App* app, std::string name) : AppTab(app, name) { }

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
        ImGui::Text(m_app->m_led_controller.connection_status_str().c_str());
        if (!m_app->m_led_controller.is_scanning())
        {
            if (!m_app->m_led_controller.is_connected())
            {
                if (ImGui::InputText("Device Name", m_app->m_led_controller.name, IM_ARRAYSIZE(m_app->m_led_controller.name)));
                if (ImGui::Button("Connect"))
                {
                    m_app->m_led_controller.scan_and_connect();
                }
            }
        }
    }
    ImGui::End();

    if (ImGui::Begin("Light Configs")) 
    {
    }
    ImGui::End();

    if (ImGui::Begin("Light Settings"))
    {
        ImGui::Text("Device Status");
        if (ImGui::Button(m_app->m_led_controller.is_device_on() ? "On" : "Off"))
        {
            m_app->m_led_controller.toggle_device();
        }
        ImGui::Text("Color Selection");
        if (ImGui::ColorEdit3("Color", m_app->m_led_controller.color))
        {
            m_app->m_led_controller.update_rgb();
        }
        if (ImGui::SliderFloat("Brightness", &m_app->m_led_controller.brightness, 0, 1))
        {
            m_app->m_led_controller.update_rgb();
        }

        ImGui::Text("Mode Selection");
        if (ImGui::Combo("Mode", &m_app->m_led_controller.mode.index, m_app->m_led_controller.mode.mode_strings, IM_ARRAYSIZE(m_app->m_led_controller.mode.mode_strings)))
        {
            m_app->m_led_controller.update_mode();
        }
        if (ImGui::SliderFloat("Speed", &m_app->m_led_controller.mode.speed, 0, 1))
        {
            m_app->m_led_controller.update_mode();
        }
    }
    ImGui::End();
}