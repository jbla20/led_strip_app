#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <array>

#include "simpleble/SimpleBLE.h"

enum BLESTATUS {
	UNDEFINED,
	SCANNING,
	CONNECTED,
	FAILED_TO_CONNECT,
	BLE_PERIPHERAL_NOT_FOUND,
	BLE_PERIPHERAL_NOT_CONNECTED,
	BLT_NOT_ENABLED,
};

class LEDController
{
public:
	LEDController();
	~LEDController();

	void scan_and_connect();
	void toggle_device();
	void update_rgb();
	void update_mode();
	void try_join_scanning_thread();
	std::string connection_status_str();
	bool is_connected();
	inline bool is_scanning() const { return m_is_scanning; }
	inline bool is_device_on() const { return m_is_device_on; }
	inline void set_device_on_flag(bool isOn) { m_is_device_on = isOn; }

private:
	void on_connection();
	void set_device_on(bool isOn);
	void scan_and_connect_internal();
	void write_command(SimpleBLE::ByteArray& command);

	class Mode
	{
	public:
		Mode() = default;
		Mode(int index, float speed) : index(index), speed(speed) {}

		inline char get_mode() { return (index >= 0 && index < mode_bytes.size()) ? mode_bytes.at(index) : 0; };

	public:
		int index;
		float speed;

		static inline const char* mode_strings[] = { "None",
			"Seven color cross fade", "Red gradual change", "Green gradual change", "Blue gradual change", "Yellow gradual change",
			"Cyan gradual change", "Purple gradual change", "White gradual change", "Red, Green cross fade", "Red blue cross fade",
			"Green blue cross fade", "Seven color strobe flash", "Red strobe flash", "Green strobe flash", "Blue strobe flash",
			"Yellow strobe flash", "Cyan strobe flash", "Purple strobe flash", "White strobe flash", "Seven color jumping change"
		};

	private:
		static inline const std::array<char, 21> mode_bytes = { 0x00,
			0x25, 0x26, 0x27, 0x28, 0x29,
			0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
			0x2f, 0x30, 0x31, 0x32, 0x33,
			0x34, 0x35, 0x36, 0x37, 0x38
		};
	};

public:
	char name[128];
	float* color;
	float brightness;
	Mode mode;

private:
	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::ByteArray TURN_ON_COMMAND = { (char)0xCC, (char)0x23, (char)0x33 };
	const SimpleBLE::ByteArray TURN_OFF_COMMAND = { (char)0xCC, (char)0x24, (char)0x33 };
	SimpleBLE::ByteArray color_command = { (char)0x56, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0xF0, (char)0xAA };
	SimpleBLE::ByteArray mode_command = { (char)0xBB, (char)0x00, (char)0x00, (char)0x44 };

	SimpleBLE::Peripheral* m_peripheral;
	std::atomic_bool m_is_scanning;
	bool m_is_device_on;
	BLESTATUS m_connection_status;
	std::thread m_scanning_thread;
};