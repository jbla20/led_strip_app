#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <array>
#include <memory>

#include "simpleble/SimpleBLE.h"
#include "led_configuration.h"
#include "timer_configuration.h"

enum BLESTATUS {
	UNDEFINED,
	SCANNING,
	CONNECTED,
	FAILED_TO_CONNECT,
	BLE_PERIPHERAL_NOT_FOUND,
	BLE_PERIPHERAL_NOT_CONNECTED,
	BLT_NOT_ENABLED,
};

class App;

class LEDController
{
public:
	explicit LEDController(App* app, std::string name, bool timer_enabled);
	~LEDController();
	
	void scan_and_connect();
	void toggle_device();
	void update_rgb();
	void update_mode();
	void update_all();
	void try_join_scanning_thread();
	std::string connection_status_str();
	bool is_connected();
	inline bool is_scanning() const { return m_is_scanning; }
	inline bool is_device_on() { return led_config()->device_on; }

	LEDConfiguration* led_config();
	TimerConfiguration* timer_config();

private:
	void set_device_on(bool on);
	void scan_and_connect_internal();
	void write_command(SimpleBLE::ByteArray& command);

public:
	std::string m_name;
	std::string m_alias;
	bool m_timer_enabled;
	App* m_app;

private:
	// Commands
	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::ByteArray TURN_ON_COMMAND = { (char)0xCC, (char)0x23, (char)0x33 };
	const SimpleBLE::ByteArray TURN_OFF_COMMAND = { (char)0xCC, (char)0x24, (char)0x33 };
	SimpleBLE::ByteArray color_command = { (char)0x56, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0xF0, (char)0xAA };
	SimpleBLE::ByteArray mode_command = { (char)0xBB, (char)0x00, (char)0x00, (char)0x44 };

	// Bluetooth Connection
	SimpleBLE::Peripheral* m_peripheral;
	BLESTATUS m_connection_status;
	std::atomic_bool m_is_scanning;
	std::atomic_bool m_is_writing_command;
	std::thread m_scanning_thread;
	std::thread m_command_thread;
};