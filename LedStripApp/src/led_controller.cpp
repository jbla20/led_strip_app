#include "led_controller.h"
#include "app.h"
#include "simpleble/Exceptions.h"
#include <iostream>
#include <algorithm>

LEDController::LEDController(App* app, std::string name, bool timer_enabled) 
    : m_app(app), m_name(name), m_alias(m_name), m_timer_enabled(timer_enabled)
{
    m_connection_status = BLESTATUS::UNDEFINED;
    m_is_scanning = false;
    m_peripheral = nullptr;
}

LEDController::~LEDController()
{
    if (m_scanning_thread.joinable())
    {
        m_scanning_thread.join();
    }
    if (m_command_thread.joinable())
    {
        m_command_thread.join();
    }
    if (m_peripheral != nullptr) 
    {
        m_peripheral->disconnect();
        delete m_peripheral;
        m_peripheral = nullptr; // Avoid dangling pointer
    }
}

void LEDController::scan_and_connect()
{
    if (m_is_scanning || is_connected())
    {
        return;
    }
    m_scanning_thread = std::thread(&LEDController::scan_and_connect_internal, this);
}

void LEDController::toggle_device()
{
    led_config()->device_on = !led_config()->device_on;
    set_device_on(led_config()->device_on);
}

void LEDController::write_command(SimpleBLE::ByteArray& command)
{
    if (!is_connected())
    {
        m_connection_status = BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED;
        std::cout << "[Warning] Cannot write to unconnected controller \'" << m_name << "\'." << std::endl;
        return;
    }
    
    if (m_is_writing_command.load())
    {
        std::cout << "[Debug] Writing command is busy. Skipping for \'" << m_name << "\'." << std::endl;
        return;
    }

    m_is_writing_command.store(true);
    if (m_command_thread.joinable()) {
        m_command_thread.join(); // Wait for the previous thread to finish
    }
    m_command_thread = std::thread([this, command]() {
        try
        {
            m_peripheral->write_request(WRITE_SERVICE, WRITE_CHARACTERISTIC, command);
            std::cout << "[Debug] Command successfully written to LED controller." << std::endl;
        }
        catch (const SimpleBLE::Exception::BaseException& e)
        {
            std::cerr << "[Error] Exception during write request: " << e.what() << std::endl;
        }
        m_is_writing_command.store(false);
    });
}

bool LEDController::is_connected()
{
    return m_peripheral != nullptr && m_peripheral->is_connected();
}

void LEDController::try_join_scanning_thread()
{
    if (m_is_scanning || !m_scanning_thread.joinable())
    {
        return;
    }
    m_scanning_thread.join();
}

std::string LEDController::connection_status_str()
{
    std::string str = "";
    switch (m_connection_status)
    {
    case BLESTATUS::UNDEFINED:
        str = "No Status";
        break;
    case BLESTATUS::SCANNING:
        str = "Scanning for device...";
        break;
    case BLESTATUS::CONNECTED:
        str = "Connected!";
        break;
    case BLESTATUS::FAILED_TO_CONNECT:
        str = "Failed to connect!";
        break;
    case BLESTATUS::BLE_PERIPHERAL_NOT_FOUND:
        str = "Could not find the peripheral!";
        break;
    case BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED:
        str = "Peripheral is not connected!";
        break;
    case BLESTATUS::BLT_NOT_ENABLED:
        str = "Bluetooth is not enabled!";
        break;
    }
    
    return str;
}

void LEDController::update_all()
{
    set_device_on(led_config()->device_on);
    update_rgb();
    update_mode();
}

void LEDController::set_device_on(bool on)
{
    SimpleBLE::ByteArray command = on ? TURN_ON_COMMAND : TURN_OFF_COMMAND;
    write_command(command);
}

void LEDController::update_rgb()
{
    color_command[1] = static_cast<char>(led_config()->color[0] * led_config()->brightness * 255.000);
    color_command[2] = static_cast<char>(led_config()->color[1] * led_config()->brightness * 255.000);
    color_command[3] = static_cast<char>(led_config()->color[2] * led_config()->brightness * 255.000);
    write_command(color_command);
}

void LEDController::update_mode()
{
    using Range = std::pair<float, float>;
    auto interp1d = [](Range range_in, Range range_out, float input) -> float {
        input = std::clamp(input, range_in.first, range_in.second);
        return range_out.first + (input - range_in.first) * (range_out.second - range_out.first) / (range_in.second - range_in.first);
    };

    mode_command[1] = static_cast<char>(led_config()->mode.get_mode());
    mode_command[2] = static_cast<char>(interp1d({ 0.0, 1.0 }, { 1.0, 31.0 }, 1 - led_config()->mode.speed));
    write_command(mode_command);
}

void LEDController::scan_and_connect_internal()
{
    m_is_scanning = true;
    m_connection_status = BLESTATUS::SCANNING;
    std::cout << "[Info] Scanning for device..." << std::endl;

    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    SimpleBLE::Adapter adapter = adapters[0];
    if (!adapter.bluetooth_enabled())
    {
        m_is_scanning = false;
        m_connection_status = BLESTATUS::BLT_NOT_ENABLED;
        std::cout << "[Warning] Bluetooth is not enabled!" << std::endl;
        return;
    }

    SimpleBLE::Peripheral peri;
    bool device_found = false;
    char* d_name = m_name.data();
    adapter.set_callback_on_scan_found([&d_name, &peri, &device_found](SimpleBLE::Peripheral peripheral) mutable {
        if (peripheral.identifier().compare(d_name) != 0)
        {
            return;
        }

        device_found = true;
        peri = peripheral;
        });

    adapter.scan_start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    adapter.scan_stop();

    if (!device_found)
    {
        m_connection_status = BLESTATUS::BLE_PERIPHERAL_NOT_FOUND;
        std::cout << "[Error] Could not find the peripheral!" << std::endl;
    }
    else
    {
        m_peripheral = new SimpleBLE::Peripheral(peri);
        m_peripheral->connect();

        if (is_connected())
        {
            m_connection_status = BLESTATUS::CONNECTED;
            std::cout << "[Info] Connected to controller \'" << m_name << "\'." << std::endl;
            update_all();
        }
        else
        {
            m_connection_status = BLESTATUS::FAILED_TO_CONNECT;
            std::cout << "[Error] Failed to connect to controller \'" << m_name << "\'." << std::endl;
        }
    }
    m_is_scanning = false;
}

LEDConfiguration* LEDController::led_config()
{
    try
    {
        int index = m_app->m_selected_led_configs.at(m_name);
        return m_app->m_led_configs.at(index).get();
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "Failed to find led config for controller \'" << m_name << "\': " << e.what() << std::endl;
        return nullptr;
    }
}

TimerConfiguration* LEDController::timer_config()
{
    try
    {
        int index = m_app->m_selected_timer_configs.at(m_name);
        return m_app->m_timer_configs.at(index).get();
    }
    catch (const std::out_of_range& e)
    {
        std::cout << "Failed to find timer config for controller \'" << m_name << "\': " << e.what() << std::endl;
        return nullptr;
    }
}

