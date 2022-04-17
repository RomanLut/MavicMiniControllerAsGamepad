#pragma once

#include <Windows.h>
#include <stdio.h>

#define HID_USAGE_X		0x30
#define HID_USAGE_Y		0x31
#define HID_USAGE_Z		0x32
#define HID_USAGE_RX	0x33
#define HID_USAGE_RY	0x34
#define HID_USAGE_RZ	0x35
#define HID_USAGE_SL0	0x36
#define HID_USAGE_SL1	0x37

class Joystick
{
private:
	bool button_1, button_2, button_3, button_4, button_5, button_6;
	bool connected;
	unsigned int interfaceId;
	bool log = false;
	int thrustValue;
	int thrustSpeed;
	int cameraValue;
	int cameraSpeed;
	bool lastButtonHome;
	bool lastButtonLB;
	bool lastButtonRB;

	bool switchHome;
	bool switchLB;
	bool switchRB;
	int homeValue;
public:
	Joystick(int interfaceId, int logging);
	~Joystick();
	void update(int l_hor, int l_ver, int r_hor, int r_ver, int camera, bool buttonLB, bool buttonRB, bool buttonHome);
	bool isConnected() { return connected;  }
	void tick();
};