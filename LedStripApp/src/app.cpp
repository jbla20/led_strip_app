#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ranges>
#include "app.h"
#include "helpers.h"
#include "yaml-cpp/yaml.h"
#include "imgui.h"
#include "imgui_internal.h"

#pragma comment(lib, "shell32.lib")

App::App() : m_window(L"LED Strip Controller"), m_timer(this)
{
    std::string name = "Default";
    m_led_controllers.emplace_back(std::make_unique<LEDController>(this, name, true));
    m_selected_controller = 0;

    m_led_configs.emplace_back(std::make_unique<LEDConfiguration>(name, false, std::array<float, 3>{1.0f, 1.0f, 1.0f}, 1.0f, Mode(0, 0.0f)));
    m_selected_led_configs = { { name, 0 } };

    m_timer_configs.emplace_back(std::make_unique<TimerConfiguration>(name, 0.0f, 0.0f, 0, false));
    m_selected_timer_configs = { { name, 0 } };
}

App::~App() {
    save_settings();
    std::ranges::for_each(m_led_controllers, [](const std::unique_ptr<LEDController>& controller)
        { if (controller->is_device_on()) controller->toggle_device(); }
    );
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

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
        led_controller()->try_join_scanning_thread();
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

    // Construct the settings.yaml file path
    std::wstring settings_file = path + L"\\settings.yaml";

    // Open the YAML file
    try
    {
        // Construct the settings.yaml file path
        std::ifstream file(settings_file);
        if (!file.is_open())
        {
            save_settings();  // If file doesn't exist, save current settings
            return;
        }

        YAML::Node settings = YAML::Load(file);  // Load YAML from file

        // Load LED controllers
        if (settings["controllers"])
        {
            m_led_controllers.resize(1 + settings["controllers"].size());
            for (size_t i = 1; i < m_led_controllers.size(); i++)
            {
                // Predefine parameters needed to load LED controller
                std::string name = "\0";
                int selected_led_config = 0;
                int selected_timer_config = 0;
                bool timer_enabled = true;

                // Load values
                const YAML::Node& controller_yaml = settings["controllers"][i];
                if (controller_yaml["name"])
                    name = controller_yaml["name"].as<std::string>();

                if (controller_yaml["selected_led_config"])
                    selected_led_config = controller_yaml["selected_led_config"].as<int>();

                if (controller_yaml["selected_timer_config"])
                    selected_timer_config = controller_yaml["selected_timer_config"].as<int>();

                if (controller_yaml["timer_enabled"])
                    timer_enabled = controller_yaml["timer_enabled"].as<bool>();

                // Create controller
                m_led_controllers[i] = std::make_unique<LEDController>(this, name, timer_enabled);
                m_selected_led_configs[name] = selected_led_config;
                m_selected_timer_configs[name] = selected_timer_config;
            }
        }

        // Load LED configurations
        if (settings["led_configs"])
        {
            m_led_configs.resize(1 + settings["led_configs"].size());
            for (size_t i = 1; i < m_led_configs.size(); i++)
            {
                // Predefine parameters needed to load LED configuration
                std::string name = "\0";
                bool device_on = false;
                std::array<float, 3> color = { 1.0f, 1.0f, 1.0f };
                float brightness = 1.0f;
                Mode mode = { 0, 0.0f };

                // Load values
                const YAML::Node& led_config_yaml = settings["led_configs"][i];
                if (led_config_yaml["name"])
                    name = led_config_yaml["name"].as<std::string>();

                if (led_config_yaml["device_on"])
                    device_on = led_config_yaml["device_on"].as<bool>();

                if (led_config_yaml["color"])
                {
                    const YAML::Node& color_yaml = led_config_yaml["color"];
                    if (color_yaml.size() == 3)
                    {
                        color[0] = color_yaml[0].as<float>();
                        color[1] = color_yaml[1].as<float>();
                        color[2] = color_yaml[2].as<float>();
                    }
                }

                if (led_config_yaml["brightness"])
                    brightness = led_config_yaml["brightness"].as<float>();

                if (led_config_yaml["mode"])
                {
                    const YAML::Node& mode_yaml = led_config_yaml["mode"];
                    if (mode_yaml["index"])
                        mode.index = mode_yaml["index"].as<int>();
                    if (mode_yaml["speed"])
                        mode.speed = mode_yaml["speed"].as<float>();
                }

                // Load led configuration
                m_led_configs[i] = std::make_unique<LEDConfiguration>(name, device_on, color, brightness, mode);
            }
        }

        if (settings["timer_configs"])
        {
            m_timer_configs.resize(1 + settings["timer_configs"].size());
            for (size_t i = 1; i < m_timer_configs.size(); i++)
            {
                // Predefine parameters needed to load timer configuration
                std::string name = "\0";
                float start = 0.0f;
                float end = 10.0f;
                int repeat = 1;
                bool inverse = false;

                // Load values
                const YAML::Node& timer_config_yaml = settings["timer_configs"][i];
                if (timer_config_yaml["name"])
                    name = timer_config_yaml["name"].as<std::string>();

                if (timer_config_yaml["start"])
                    start = timer_config_yaml["start"].as<float>();

                if (timer_config_yaml["end"])
                    end = timer_config_yaml["end"].as<float>();

                if (timer_config_yaml["repeat"])
                    repeat = timer_config_yaml["repeat"].as<int>();

                if (timer_config_yaml["inverse"])
                    inverse = timer_config_yaml["inverse"].as<bool>();

                // Load timer configuration
                m_timer_configs[i] = std::make_unique<TimerConfiguration>(name, start, end, repeat, inverse);
            }
        }

        file.close();
        std::cout << "Loaded settings" << std::endl;
    }
    catch (const YAML::Exception& ex)
    {
        // Handle YAML exceptions (e.g., file errors, parsing errors)
        std::cerr << "Failed to load settings: " << ex.what() << "\n";
    }
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

    // Construct the settings.yaml file path
    std::wstring settings_file = path + L"\\settings.yaml";

    try
    {
        // Create a YAML node and populate it with settings
        YAML::Node settings;

        for (size_t i = 1; i < m_led_controllers.size(); i++)
        {
            settings["controllers"][i]["name"] = m_led_controllers[i]->m_name;
            settings["controllers"][i]["selected_led_config"] = m_selected_led_configs[m_led_controllers[i]->m_name];
            settings["controllers"][i]["selected_timer_config"] = m_selected_timer_configs[m_led_controllers[i]->m_name];
            settings["controllers"][i]["timer_enabled"] = m_led_controllers[i]->m_timer_enabled;
        }
        
        for (size_t i = 1; i < m_led_configs.size(); i++)
        {
            settings["led_configs"][i]["name"] = m_led_configs[i]->name;
            settings["led_configs"][i]["device_on"] = m_led_configs[i]->device_on;
            settings["led_configs"][i]["color"] = YAML::Node(YAML::NodeType::Sequence); // List for color values
            settings["led_configs"][i]["color"].push_back(m_led_configs[i]->color[0]);
            settings["led_configs"][i]["color"].push_back(m_led_configs[i]->color[1]);
            settings["led_configs"][i]["color"].push_back(m_led_configs[i]->color[2]);
            settings["led_configs"][i]["brightness"] = m_led_configs[i]->brightness;
            settings["led_configs"][i]["mode"]["index"] = m_led_configs[i]->mode.index;
            settings["led_configs"][i]["mode"]["speed"] = m_led_configs[i]->mode.speed;
        }

        for (size_t i = 1; i < m_timer_configs.size(); i++)
        {
            settings["timer_configs"][i]["name"] = m_timer_configs[i]->name;
            settings["timer_configs"][i]["start"] = m_timer_configs[i]->start;
            settings["timer_configs"][i]["end"] = m_timer_configs[i]->end;
            settings["timer_configs"][i]["repeat"] = m_timer_configs[i]->repeat;
            settings["timer_configs"][i]["inverse"] = m_timer_configs[i]->inverse;
        }

        // Save the YAML node to the file
        std::ofstream file(settings_file);
        if (!file.is_open())
        {
            return;
        }
        file << settings; // Write YAML to the file
        std::cout << "Saved settings." << std::endl;
    }
    catch (const YAML::Exception& ex)
    {
        // Handle YAML exceptions (e.g., serialization errors)
        std::cerr << "Failed to save settings: " << ex.what() << "\n";
    }
}

bool App::create_new_controller(std::string name)
{
    m_led_controllers.emplace_back(std::make_unique<LEDController>(this, name, true));
    m_selected_led_configs[name] = 0;
    m_selected_timer_configs[name] = 0;
    return true;
}

bool App::update_controller(int index)
{
    try
    {
        if (index < 0 || index >= m_led_controllers.size())
        {
            throw std::out_of_range("Index " + std::to_string(index) + " is out of range");
        }

        m_selected_controller = index;
    }
    catch (std::out_of_range& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

bool App::rename_selected_controller(std::string new_name)
{
    led_controller()->m_alias = new_name;
    return true;
}

bool App::delete_selected_controller()
{
    try
    {
        std::optional<int> index = helpers::index_in_vector(led_controller_names(), led_controller()->m_name);

        if (!index)
        {
            throw std::runtime_error("Controller not found");
        }

        *index += 1; // To account for default controller
        if (m_led_controllers[*index]->is_device_on())
        {
            m_led_controllers[*index]->toggle_device();
        }
        m_selected_led_configs.erase(m_led_controllers[*index]->m_name);
        m_led_controllers.erase(m_led_controllers.begin() + *index);
        m_selected_controller = 0;
    }
    catch (std::out_of_range& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

bool App::create_new_led_config(std::string name)
{
    m_led_configs.emplace_back(std::make_unique<LEDConfiguration>(*led_controller()->led_config()));
    m_led_configs.back()->name = name;
    return true;
}

bool App::update_controller_led_config(int index)
{
    try
    {
        m_selected_led_configs.at(led_controller()->m_name) = index;
        led_controller()->update_all();
    }
    catch (std::out_of_range& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

bool App::rename_selected_led_config(std::string new_name)
{
    led_controller()->led_config()->name = new_name;
    return true;
}

bool App::delete_selected_led_config()
{
    try
    {
        std::optional<int> index = helpers::index_in_vector(led_config_names(), led_controller()->led_config()->name);

        if (!index)
        {
            throw std::runtime_error("Led config not found");
        }

        *index += 1; // To account for default config
        m_led_configs.erase(m_led_configs.begin() + *index);
        for (size_t i = 0; i < m_led_controllers.size(); i++)
        {
            if (m_selected_led_configs[m_led_controllers[i]->m_name] == *index)
            {
                m_selected_led_configs[m_led_controllers[i]->m_name] = 0;
            }
            else if (m_selected_led_configs[m_led_controllers[i]->m_name] > *index)
            {
                m_selected_led_configs[m_led_controllers[i]->m_name] -= 1;
            }

        }
        led_controller()->update_all();
        return true;
    }
    catch (std::runtime_error& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

bool App::create_new_timer_config(std::string name)
{
    m_timer_configs.emplace_back(std::make_unique<TimerConfiguration>(*led_controller()->timer_config()));
    m_timer_configs.back()->name = name;
    return true;
}

bool App::update_controller_timer_config(int index)
{
    try
    {
        m_selected_timer_configs.at(led_controller()->m_name) = index;
    }
    catch (std::out_of_range& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

bool App::rename_selected_timer_config(std::string new_name)
{
    led_controller()->timer_config()->name = new_name;
    return true;
}

bool App::delete_selected_timer_config()
{
    try
    {
        std::optional<int> index = helpers::index_in_vector(timer_config_names(), led_controller()->timer_config()->name);

        if (!index)
        {
            throw std::runtime_error("Timer config not found");
        }

        *index += 1; // To account for default config
        m_timer_configs.erase(m_timer_configs.begin() + *index);
        for (size_t i = 0; i < m_led_controllers.size(); i++)
        {
            if (m_selected_timer_configs[m_led_controllers[i]->m_name] == *index)
            {
                m_selected_timer_configs[m_led_controllers[i]->m_name] = 0;
            }
            else if (m_selected_timer_configs[m_led_controllers[i]->m_name] > *index)
            {
                m_selected_timer_configs[m_led_controllers[i]->m_name] -= 1;
            }

        }
        return true;
    }
    catch (std::runtime_error& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}


LEDController* App::led_controller()
{
    try
    {
        return m_led_controllers.at(m_selected_controller).get();
    }
    catch (std::out_of_range& err)
    {
        std::cout << err.what() << std::endl;
        return nullptr;
    }
}

std::vector<std::string> App::led_controller_names()
{
    std::vector<std::string> names;
    std::ranges::transform(m_led_controllers, std::back_inserter(names), [](std::unique_ptr<LEDController>& controller) 
        { return controller->m_name; }
    );
    names.erase(names.begin()); // TODO: Smarter way to ignore first element
    return names;
}

std::vector<std::string> App::led_controller_aliases()
{
    std::vector<std::string> names;
    std::ranges::transform(m_led_controllers, std::back_inserter(names), [](std::unique_ptr<LEDController>& led_controller)
        { return led_controller->m_alias; }
    );
    names.erase(names.begin()); // TODO: Smarter way to ignore first element
    return names;
}

std::vector<std::string> App::led_config_names()
{
    std::vector<std::string> names;
    std::ranges::transform(m_led_configs, std::back_inserter(names), [](std::unique_ptr<LEDConfiguration>& led_config) 
        { return led_config->name; }
    );
    names.erase(names.begin()); // TODO: Smarter way to ignore first element
    return names;
}

std::vector<std::string> App::timer_config_names()
{
    std::vector<std::string> names;
    std::ranges::transform(m_timer_configs, std::back_inserter(names), [](std::unique_ptr<TimerConfiguration>& timer_config)
        { return timer_config->name; }
    );
    names.erase(names.begin()); // TODO: Smarter way to ignore first element
    return names;
}
