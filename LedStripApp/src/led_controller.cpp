#include "led_controller.h"
#include "simpleble/Exceptions.h"
#include <iostream>
#include <algorithm>

LEDController::LEDController()
{
    color = new float[3](1.0f, 1.0f, 1.0f);
    brightness = 1.0f;
    mode = Mode(0, 0.0f);
    m_connection_status = BLESTATUS::UNDEFINED;
    m_is_scanning = false;
    m_is_device_on = false;
    m_peripheral = nullptr;
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
    m_is_device_on = !m_is_device_on;
    set_device_on(m_is_device_on);
}

void LEDController::write_command(SimpleBLE::ByteArray& command)
{
    if (!is_connected())
    {
        m_connection_status = BLESTATUS::BLE_PERIPHERAL_NOT_CONNECTED;
        return;
    }

    try {
        m_peripheral->write_request(WRITE_SERVICE, WRITE_CHARACTERISTIC, command);
    }
    catch (SimpleBLE::Exception::BaseException ex)
    {
#if DEBUG
        std::cerr << ex.what() << std::endl;
#endif // DEBUG
    }
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

void LEDController::on_connection()
{
    set_device_on(m_is_device_on);
    update_rgb();
    update_mode();
}

void LEDController::set_device_on(bool isOn)
{
    SimpleBLE::ByteArray command = isOn ? TURN_ON_COMMAND : TURN_OFF_COMMAND;
    write_command(command);
}

void LEDController::update_rgb()
{
    color_command[1] = static_cast<char>(color[0] * brightness * 255.000);
    color_command[2] = static_cast<char>(color[1] * brightness * 255.000);
    color_command[3] = static_cast<char>(color[2] * brightness * 255.000);
    write_command(color_command);
}

void LEDController::update_mode()
{
    using Range = std::pair<float, float>;
    auto interp1d = [](Range range_in, Range range_out, float input) {
        input = std::clamp(input, range_in.first, range_in.second);
        return range_out.first + (input - range_in.first) * (range_out.second - range_out.first) / (range_in.second - range_in.first);
    };

    mode_command[1] = static_cast<char>(mode.get_mode());
    mode_command[2] = static_cast<char>(interp1d({ 0.0, 1.0 }, { 1.0, 31.0 }, 1 - mode.speed));
    write_command(mode_command);
}

void LEDController::scan_and_connect_internal()
{
    m_is_scanning = true;
    m_connection_status = BLESTATUS::SCANNING;

    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    SimpleBLE::Adapter adapter = adapters[0];
    if (!adapter.bluetooth_enabled())
    {
        m_is_scanning = false;
        m_connection_status = BLESTATUS::BLT_NOT_ENABLED;
        return;
    }

    SimpleBLE::Peripheral peri;
    bool device_found = false;
    char* d_name = name;
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
    }
    else
    {
        m_peripheral = new SimpleBLE::Peripheral(peri);
        m_peripheral->connect();

        if (is_connected())
        {
            m_connection_status = BLESTATUS::CONNECTED;
            on_connection();
        }
        else
        {
            m_connection_status = BLESTATUS::FAILED_TO_CONNECT;
        }
    }
    m_is_scanning = false;
}

LEDController::~LEDController()
{
    if (m_scanning_thread.joinable())
    {
        m_scanning_thread.join();
    }
    if (m_peripheral != nullptr) 
    {
        m_peripheral->disconnect();
        delete m_peripheral;
    }
    delete[] color;
}