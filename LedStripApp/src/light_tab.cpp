#include <vector>
#include <ranges>
#include <algorithm>

#define NOMINMAX
#include "light_tab.h"
#include "app.h"
#include "helpers.h"

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
        m_first_frame = true;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, nullptr, &dockspace_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.5714f, nullptr, &dockspace_id);
        ImGuiID dock_id_center = dockspace_id;  // The remaining space in the center

        ImGui::DockBuilderDockWindow("Bluetooth Connect", dock_id_left);
        ImGui::DockBuilderDockWindow("Light Settings", dock_id_center);
        ImGui::DockBuilderDockWindow("Live Timer View", dock_id_right);
        ImGui::DockBuilderDockWindow("Timers", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }
    else if (m_first_frame)
    {
        ImGui::SetWindowFocus("Timers");
        m_first_frame = false;
    }
    
    // Creating ui for different dock windows
    if (ImGui::Begin("Bluetooth Connect")) {
        // Connect controller
        ImGui::Text(m_app->led_controller()->connection_status_str().c_str());
        if (!m_app->led_controller()->is_scanning())
        {
            if (!m_app->led_controller()->is_connected())
            {
                if (ImGui::Button("Connect"))
                {
                    std::cout << "[Info] Connecting to controller \'" << m_app->led_controller()->m_name << "\'." << std::endl;
                    m_app->led_controller()->scan_and_connect();
                }
            }
        }

        // Known devices
        std::vector<const char*> controller_items;
        controller_items.reserve(controller_items.size());
        std::vector<std::string> controller_names = m_app->led_controller_aliases();
        for (const auto& item : controller_names)
        {
            controller_items.push_back(item.c_str());
        }
        ImGui::Text("Known devices");
        if (ImGui::ListBox(" ", &m_selected_controller, controller_items.data(), controller_items.size(), 10))
        {
            m_app->update_controller(m_selected_controller + 1);
        }

        // Create device
        ImGui::Text("New device");
        ImGui::InputText("##New device", m_new_controller_name, sizeof(m_new_controller_name), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        if (ImGui::Button("Create"))
        {
            bool name_exists = helpers::exists_in_vector(m_app->led_controller_names(), std::string(m_new_controller_name)) || helpers::exists_in_vector(m_app->led_controller_aliases(), std::string(m_new_controller_name));
            if (m_new_controller_name[0] != '\0' && !name_exists)
            {
                if (m_app->create_new_controller(std::string(m_new_controller_name)))
                {
                    std::cout << "[Info] Creating new controller \'" << m_new_controller_name << "\'." << std::endl;
                    m_app->update_controller(static_cast<int>(m_app->m_led_controllers.size()) - 1);
                    m_selected_controller = static_cast<int>(m_app->m_led_controllers.size()) - 1;
                }
            }
        }

        // Rename device
        ImGui::Text("Rename device");
        ImGui::InputText("##Rename device", m_rename_controller_name, sizeof(m_rename_controller_name), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            bool name_exists = helpers::exists_in_vector(m_app->led_controller_names(), std::string(m_rename_controller_name)) || helpers::exists_in_vector(m_app->led_controller_aliases(), std::string(m_rename_controller_name));
            if (m_rename_controller_name[0] != '\0' && !name_exists)
            {
                std::cout << "[Info] Renaming controller from \'" << m_app->led_controller()->m_alias << "\' to \'" << m_new_controller_name << "\'." << std::endl;
                m_app->rename_selected_controller(std::string(m_rename_controller_name));
            }
        }

        // Delete and reset device
        if (ImGui::Button("Delete"))
        {
            std::cout << "[Info] Deleting controller \'" << m_app->led_controller()->m_alias << "\' with alias \'" << m_app->led_controller()->m_name << "\'." << std::endl;
            m_app->delete_selected_controller();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            std::cout << "[Info] Resetting controller name from \'" << m_app->led_controller()->m_alias << "\' to \'" << m_app->led_controller()->m_name << "\'." << std::endl;
            m_app->led_controller()->m_alias = m_app->led_controller()->m_name;
        }

    }
    ImGui::End(); // Bluetooth Connect

    if (ImGui::Begin("Light Settings"))
    {
        // Available configs
        std::vector<const char*> led_config_items;
        led_config_items.reserve(led_config_items.size());
        std::vector<std::string> led_config_names = m_app->led_config_names();
        for (const auto& item : led_config_names)
        {
            led_config_items.push_back(item.c_str());
        }
        ImGui::Text("Available led configs");
        if (ImGui::ListBox(" ", &m_selected_led_config, led_config_items.data(), led_config_items.size(), 10))
        {
            m_app->update_controller_led_config(m_selected_led_config + 1);
        }

        // New config
        ImGui::Text("New led config");
        ImGui::InputText("##New led config", m_new_led_config_name, sizeof(m_new_led_config_name), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        if (ImGui::Button("Create"))
        {
            if (m_new_led_config_name[0] != '\0' && !helpers::exists_in_vector(m_app->led_config_names(), std::string(m_new_led_config_name)))
            {
                if (m_app->create_new_led_config(std::string(m_new_led_config_name)))
                {
                    std::cout << "[Info] Creating new led config \'" << m_new_led_config_name << "\' for controller \'" << m_app->led_controller()->m_name << "\'." << std::endl;
                    m_app->update_controller_led_config(static_cast<int>(m_app->m_led_configs.size()) - 1);
                    m_selected_led_config = static_cast<int>(m_app->m_led_configs.size()) - 1;
                }
            }
        }

        // Rename config
        ImGui::Text("Rename led config");
        ImGui::InputText("##Rename led config", m_rename_led_config_name, sizeof(m_rename_led_config_name), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            if (m_rename_led_config_name[0] != '\0' && !helpers::exists_in_vector(m_app->led_config_names(), std::string(m_rename_led_config_name)))
            {
                std::cout << "[Info] Renaming led config from \'" << m_app->led_controller()->led_config()->name << "\' to \'" << m_rename_led_config_name << "\'." << std::endl;
                m_app->rename_selected_led_config(std::string(m_rename_led_config_name));
            }
        }

        // Delete config
        if (ImGui::Button("Delete"))
        {
            std::cout << "[Info] Deleting led config \'" << m_app->led_controller()->led_config()->name << "\'." << std::endl;
            m_app->delete_selected_led_config();
        }

        ImGui::NewLine();
        ImGui::Separator();

        // Light settings
        ImGui::Text("Device status");
        if (ImGui::Button(m_app->led_controller()->is_device_on() ? "On" : "Off"))
        {
            m_app->led_controller()->toggle_device();
        }
        ImGui::Text("Color selection");
        if (ImGui::ColorEdit3("Color", m_app->led_controller()->led_config()->color.data()))
        {
            m_app->led_controller()->update_rgb();
        }
        if (ImGui::SliderFloat("Brightness", &m_app->led_controller()->led_config()->brightness, 0, 1))
        {
            m_app->led_controller()->update_rgb();
        }
        ImGui::Text("Mode selection");
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

    if (ImGui::Begin("Timers"))
    {
        ImGui::Checkbox("Timer enabled", &m_app->led_controller()->m_timer_enabled);
        if (m_app->led_controller()->m_timer_enabled)
        {
            // Available configs
            std::vector<const char*> timer_config_items;
            timer_config_items.reserve(timer_config_items.size());
            std::vector<std::string> timer_config_names = m_app->timer_config_names();
            for (const auto& item : timer_config_names)
            {
                timer_config_items.push_back(item.c_str());
            }
            ImGui::Text("Available timer configs");
            if (ImGui::ListBox(" ", &m_selected_timer_config, timer_config_items.data(), timer_config_items.size(), 10))
            {
                m_app->update_controller_timer_config(m_selected_timer_config + 1);
            }

            // New config
            ImGui::Text("New timer config");
            ImGui::InputText("##New timer config", m_new_timer_config_name, sizeof(m_new_timer_config_name), ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine();
            if (ImGui::Button("Create"))
            {
                if (m_new_timer_config_name[0] != '\0' && !helpers::exists_in_vector(m_app->timer_config_names(), std::string(m_new_timer_config_name)))
                {
                    if (m_app->create_new_timer_config(std::string(m_new_timer_config_name)))
                    {
                        std::cout << "[Info] Creating new timer config \'" << m_new_timer_config_name << "\' for controller \'" << m_app->led_controller()->m_name << "\'." << std::endl;
                        m_app->update_controller_timer_config(static_cast<int>(m_app->m_timer_configs.size()) - 1);
                        m_selected_timer_config = static_cast<int>(m_app->m_timer_configs.size()) - 1;
                    }
                }
            }

            // Rename config
            ImGui::Text("Rename timer config");
            ImGui::InputText("##Rename timer config", m_rename_timer_config_name, sizeof(m_rename_timer_config_name), ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                if (m_rename_timer_config_name[0] != '\0' && !helpers::exists_in_vector(m_app->timer_config_names(), std::string(m_rename_timer_config_name)))
                {
                    std::cout << "[Info] Renaming timer config from \'" << m_app->led_controller()->timer_config()->name << "\' to \'" << m_rename_timer_config_name << "\'." << std::endl;
                    m_app->rename_selected_timer_config(std::string(m_rename_timer_config_name));
                }
            }

            // Delete config
            if (ImGui::Button("Delete"))
            {
                std::cout << "[Info] Deleting timer config \'" << m_app->led_controller()->timer_config()->name << "\'." << std::endl;
                m_app->delete_selected_timer_config();
            }

            ImGui::NewLine();
            ImGui::Separator();

            // Timer settings
            ImGui::Text("Timer selection");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
            if (ImGui::InputFloat("Start time", &m_app->led_controller()->timer_config()->start))
            {
                if (m_app->led_controller()->timer_config()->start < 0)
                {
                    std::cout << "[Info] Start time must be non-negative" << std::endl;
                    m_app->led_controller()->timer_config()->start = 0.0f;
                }
            }
            ImGui::SameLine();
            if (ImGui::InputFloat("End time", &m_app->led_controller()->timer_config()->end))
            {
                if (m_app->led_controller()->timer_config()->end <= 0)
                {
                    std::cout << "[Info] End time must be positive" << std::endl;
                    m_app->led_controller()->timer_config()->end = 1.0f;
                }
            }
            ImGui::PopItemWidth();
            if (ImGui::InputInt("Repeat number", &m_app->led_controller()->timer_config()->repeat))
            {
                if (m_app->led_controller()->timer_config()->repeat < 1)
                {
                    std::cout << "[Info] Repeat number must be non-negative" << std::endl;
                    m_app->led_controller()->timer_config()->repeat = 1;
                }
            }
            ImGui::Checkbox("Inverse", &m_app->led_controller()->timer_config()->inverse);
        }
    }
    ImGui::End(); // Timers

    if (ImGui::Begin("Live Timer View"))
    {
        // Global timer
        ImGui::Text("Global timer");
        m_app->m_timer.update();
        std::cout << "[Debug] Relative time: " << std::to_string(m_app->m_timer.get_relative_time()) << std::endl;

        ImGui::SameLine();
        if (ImGui::Button(!m_app->m_timer.is_active() ? "Start" : (!m_app->m_timer.is_paused() ? "Pause" : "Unpause")))
        {
            std::cout << "[Info] " << (!m_app->m_timer.is_active() ? "Starting" : (!m_app->m_timer.is_paused() ? "Pausing" : "Unpausing")) << " timer." << std::endl;
            m_app->m_timer.pause(!m_app->m_timer.is_paused());
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            std::cout << "[Info] Resetting timer." << std::endl;
            m_app->m_timer.reset();
        }
        ImGui::Text("Relative time: %.3f seconds", m_app->m_timer.get_relative_time());


        // Live timer view plot
        const double x_max = std::ranges::max(
            m_app->m_timer_configs | std::views::transform([](const std::unique_ptr<TimerConfiguration>& timer_config)
                { return timer_config->end * timer_config->repeat; }
            )
        );
        const double y_min = -0.1; const double y_max = 1.2;
        ImPlot::SetNextAxesLimits(0.0, x_max, y_min, y_max, ImGuiCond_Always);
        double y_ticks[2] = { 0.0, 1.0 };

        if (ImPlot::BeginPlot("Timer View", ImVec2(-1, 0), ImPlotFlags_NoInputs | ImPlotFlags_Equal))
        {
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
            ImPlot::SetupAxisTicks(ImAxis_Y1, y_ticks, 2);

            // Plot timers
            std::vector<double> x_timer;
            std::vector<double> y_timer;
            for (size_t i = 1; i < m_app->m_timer_configs.size(); i++)
            {
                const int num_elements = 2 + 4 * m_app->m_timer_configs[i]->repeat;
                x_timer.resize(num_elements);
                y_timer.resize(num_elements);

                x_timer[0] = 0;
                y_timer[0] = static_cast<double>(m_app->m_timer_configs[i]->inverse);

                for (size_t j = 0; j < m_app->m_timer_configs[i]->repeat; j++)
                {
                    x_timer[1 + 4 * j] = m_app->m_timer_configs[i]->start + j * m_app->m_timer_configs[i]->end;
                    x_timer[2 + 4 * j] = m_app->m_timer_configs[i]->start + j * m_app->m_timer_configs[i]->end;
                    x_timer[3 + 4 * j] = m_app->m_timer_configs[i]->end + j * m_app->m_timer_configs[i]->end;
                    x_timer[4 + 4 * j] = m_app->m_timer_configs[i]->end + j * m_app->m_timer_configs[i]->end;

                    y_timer[1 + 4 * j] = static_cast<double>(m_app->m_timer_configs[i]->inverse);
                    y_timer[2 + 4 * j] = static_cast<double>(!m_app->m_timer_configs[i]->inverse);
                    y_timer[3 + 4 * j] = static_cast<double>(!m_app->m_timer_configs[i]->inverse);
                    y_timer[4 + 4 * j] = static_cast<double>(m_app->m_timer_configs[i]->inverse);
                }

                x_timer[num_elements - 1] = x_max;
                y_timer[num_elements - 1] = 0.0;

                ImPlot::PlotLine(m_app->m_timer_configs[i]->name.c_str(), x_timer.data(), y_timer.data(), num_elements);
            }

            // Plot vertical line that follows relative time
            ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(255, 255, 255, 255));
            double x_vert[2] = { m_app->m_timer.get_relative_time(), m_app->m_timer.get_relative_time() };
            double y_vert[2] = { y_min, y_max };
            ImPlot::PlotLine("", x_vert, y_vert, 2);
            ImPlot::PopStyleColor();
            ImPlot::PopStyleVar();

            ImPlot::EndPlot();
        }
    }
    ImGui::End(); // Live Timer View
}