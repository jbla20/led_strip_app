#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "app.h"
#include "imgui.h"
#include "imgui_internal.h"

#pragma comment(lib, "shell32.lib")

App::App() : m_led_controller(), m_window(L"LED Strip Controller") {}

bool App::init()
{
	if (!m_window.init())
	{
		return false;
	}
    load_settings();
	return true;
}

void App::run()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = NULL;  // To avoid storing position of windows => We control it with docking
    
    ImVec2 disp_size = io.DisplaySize;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2.0;
    style.ChildRounding = 2.0;
    style.FrameRounding = 6.0;
    style.PopupRounding = 0.0;
    style.ScrollbarRounding = 3.0;
    style.GrabRounding = 6.0;
    style.LogSliderDeadzone = 4.0;
    style.TabRounding = 2.0;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.19f, 0.19f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    while (m_window.isOpen())
    {
        m_led_controller.try_join_scanning_thread();
        render();
        m_window.render();
    }

    m_window.waitForLastSubmittedFrame();
}

void App::render()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(0,
        ImGui::GetMainViewport(),
        ImGuiDockNodeFlags_PassthruCentralNode | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, viewport->WorkSize);
    ImGui::Begin("Main window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::PopStyleVar();
    if (ImGui::BeginTabBar("Main_tab_bar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip))
    {
        for (auto tab : m_tabs)
        {
            if (ImGui::BeginTabItem(tab->name().c_str()))
            {
                if (ImGui::IsItemVisible())
                {
                    if (m_current_tab != tab)
                    {
                        m_current_tab = tab;
                        m_current_tab->on_open();
                    }
                }
                tab->render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();  // "Main_tab_bar"
    }
    ImGui::End();  // "Main window"
    
    // Rendering
    ImGui::Render();
}

std::wstring App::fetch_settings_path()
{
    // Get the path of the executable
    wchar_t exe_path[MAX_PATH];
    DWORD len = GetModuleFileName(NULL, exe_path, MAX_PATH);

    if (len == 0)
    {
        return std::wstring();
    }

    // Null-terminate the string and extract the directory
    exe_path[len] = L'\0';
    std::wstring exe_dir = exe_path;
    size_t pos = exe_dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        exe_dir = exe_dir.substr(0, pos); // Get the directory
    }

    // Append the settings folder name to the path
    std::wstring settings_path = exe_dir + L"\\LedStripApp";

    return settings_path;
}

void App::load_settings()
{
    std::wstring path = fetch_settings_path();
    if (std::empty(path))
    {
        return;
    }

    std::ifstream file(path + L"\\settings.txt");
    if (!file.is_open())
    {
        save_settings();
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::string value = line.substr(line.find_first_of("=") + 1);
        if (line.find("name=") != std::string::npos)
        {
           memcpy(m_led_controller.name, value.c_str(), value.size());
        }
        else if (line.find("on=") != std::string::npos)
        {
            m_led_controller.set_device_on_flag(std::stoi(value));
        }
        else if (line.find("color=") != std::string::npos) 
        {
            std::istringstream ss(value);
            std::string color_val;
            int i = 0;
            while (ss >> color_val)
            {   
                m_led_controller.color[i++] = std::stof(color_val);
            }
        }
        else if (line.find("brightness=") != std::string::npos)
        {
            m_led_controller.brightness = std::stof(value);
        }
        else if (line.find("mode_index=") != std::string::npos)
        {
            m_led_controller.mode.index = std::stoi(value);
        }
        else if (line.find("mode_speed=") != std::string::npos)
        {
            m_led_controller.mode.speed = std::stof(value);
        }
    }
    file.close();
}

void App::save_settings()
{
    std::wstring path = fetch_settings_path();
    if (std::empty(path))
    {
        return;
    }

    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }

    std::ofstream file(path + L"\\settings.txt", std::fstream::trunc);
    if (!file.is_open())
    {
        return;
    }
    file << "name=" << m_led_controller.name << "\n";
    file << "on=" << m_led_controller.is_device_on() << "\n";
    file << "color=" << m_led_controller.color[0] << " " << m_led_controller.color[1] << " " << m_led_controller.color[2] << "\n";
    file << "brightness=" << m_led_controller.brightness << "\n";
    file << "mode_index=" << m_led_controller.mode.index << "\n";
    file << "mode_speed=" << m_led_controller.mode.speed << "\n";

    file.close();
}


App::~App() {
    save_settings();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
