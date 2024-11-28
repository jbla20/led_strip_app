#pragma once

#include <string>
#include <array>

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

class LEDConfiguration
{
public:
	explicit LEDConfiguration(std::string name, std::array<float, 3> color, float brightness, Mode mode)
		: name(name), color(color), brightness(brightness), mode(mode) {}
	~LEDConfiguration() = default;

public:
	std::string name = "\0";
	bool device_on = false;
	std::array<float, 3> color;
	float brightness;
	Mode mode;
};
