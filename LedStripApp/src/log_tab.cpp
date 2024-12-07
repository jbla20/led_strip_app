#include "log_tab.h"
#include "app.h"

LogTab::LogTab(App* app, std::string name) : AppTab(app, name) 
{ 
    static LogStreamBuffer logBuffer(m_logger_widget);
    std::cout.rdbuf(&logBuffer);
}

void LogTab::on_open()
{
}

void LogTab::render()
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

        ImGuiID dock_id_center = dockspace_id;  // The remaining space in the center

        ImGui::DockBuilderDockWindow("Log", dock_id_center);

        ImGui::DockBuilderFinish(dockspace_id);
    }
    
    // Creating ui for different dock windows
    if (ImGui::Begin("Log"))
    {
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
        if (ImGui::Combo("Log Level", &m_log_level, log_levels, IM_ARRAYSIZE(log_levels)))
        {
            m_logger_widget.SetLogLevel(m_log_level);
        }
        ImGui::PopItemWidth();

        m_logger_widget.Draw();
    }
    ImGui::End(); // Log
}