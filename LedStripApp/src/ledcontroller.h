#pragma once

#include "simpleble/SimpleBLE.h"
#include <atomic>
#include <thread>
#include <vector>
#include <array>

enum BLESTATUS {
	UNDEFINED,
	SCANNING,
	CONNECTED,
	FAILED_TO_CONNECT,
	BLE_PERIPHERAL_NOT_FOUND,
	BLE_PERIPHERAL_NOT_CONNECTED,
	BLT_NOT_ENABLED,
};

namespace Modes {
	inline const const char* modeStrings[] = {"None",
		"Seven color cross fade", "Red gradual change", "Green gradual change", "Blue gradual change", "Yellow gradual change",
		"Cyan gradual change", "Purple gradual change", "White gradual change", "Red, Green cross fade", "Red blue cross fade",
		"Green blue cross fade", "Seven color strobe flash", "Red strobe flash", "Green strobe flash", "Blue strobe flash",
		"Yellow strobe flash", "Cyan strobe flash", "Purple strobe flash", "White strobe flash", "Seven color jumping change" 
	};

	static const std::array<char, 21> modeBytes = { 0x00, 
		0x25, 0x26, 0x27, 0x28, 0x29, 
		0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
		0x2f, 0x30, 0x31, 0x32, 0x33, 
		0x34, 0x35, 0x36, 0x37, 0x38 
	};
}

class LEDController
{
public:
	char name[128];
	float* color;
	float brightness;
	char mode;
	float modeSpeed;
private:
	const SimpleBLE::BluetoothUUID WRITE_SERVICE = "0000ffd5-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::BluetoothUUID WRITE_CHARACTERISTIC = "0000ffd9-0000-1000-8000-00805f9b34fb";
	const SimpleBLE::ByteArray TURN_ON_COMMAND = { (char)0xCC, (char)0x23, (char)0x33 };
	const SimpleBLE::ByteArray TURN_OFF_COMMAND = { (char)0xCC, (char)0x24, (char)0x33 };
	SimpleBLE::ByteArray colorCommand = { (char)0x56, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0xF0, (char)0xAA };
	SimpleBLE::ByteArray modeCommand = { (char)0xBB, (char)0x00, (char)0x00, (char)0x44 };

	SimpleBLE::Peripheral* m_Peripheral;
	std::atomic_bool m_IsScanning;
	bool m_IsDeviceOn;
	BLESTATUS m_ConnectionStatus;
	std::thread m_ScanningThread;

public:
	LEDController();
	void scanAndConnect();
	void toggleDevice();
	void updateColor();
	void updateBrightness();
	void updateMode();
	void updateModeSpeed();
	void writeCommand(SimpleBLE::ByteArray& command);
	void tryJoinScanningThread();
	std::string connectionStatusStr();
	bool isConnected();
	inline bool isScanning() const { return m_IsScanning; }
	inline bool isDeviceOn() const { return m_IsDeviceOn; }
	inline void setDeviceOnFlag(bool isOn) { m_IsDeviceOn = isOn; }
	~LEDController();

private:
	void loadSettings();
	void setDeviceOn(bool isOn);
	void scanAndConnectInternal();
	void updateColorInternal(float intensity);
	void updateModeInternal(float speed);
};