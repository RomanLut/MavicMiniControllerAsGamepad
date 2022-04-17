#include "Joystick.h"
#include "winnt.h"
#include "vjoyinterface.h"

Joystick::Joystick(int id, int logging)
{
	interfaceId = id;
	if (logging == 1) {
		log = true;
	}

	this->thrustValue = 0;
	this->thrustSpeed = 0;

	this->cameraValue = 0;
	this->cameraSpeed = 0;

	this->homeValue = 16384;
	this->lastButtonHome = false;

	this->lastButtonLB = false;
	this->lastButtonRB = false;

	this->switchHome = false;
	this->switchLB = false;
	this->switchRB = false;

	printf("Creating vJoy connection\n");

	if (!vJoyEnabled()) {
		printf("  vJoy driver not enabled: Failed Getting vJoy attributes.\n");
		return;
	} else {
		printf("  vJoy Driver found\n");
	};

	VjdStat status = GetVJDStatus(interfaceId);
	switch (status)
	{
	case VJD_STAT_OWN:
		printf("  vJoy Device %d is already owned by this feeder\n", interfaceId);
		break;
	case VJD_STAT_FREE:
		printf("  vJoy Device %d is free\n", interfaceId);
		break;
	case VJD_STAT_BUSY:
		printf("  vJoy Device %d is already owned by another feeder\n  !!! Cannot continue !!!\n", interfaceId);
		return;
	case VJD_STAT_MISS:
		printf("  vJoy Device %d is not installed or disabled\n  !!! Cannot continue !!!\n", interfaceId);
		return;
	default:
		printf("  vJoy Device %d general error\n  !!! Cannot continue !!!\n", interfaceId);
		return;
	};

	if (!AcquireVJD(interfaceId)) {
		printf("  Failed to acquire vJoy Interface number %d\n", interfaceId);
		return;
	}
	else {
		ResetVJD(interfaceId);
		connected = true;
		printf("  Acquired vJoy Interface number %d\n", interfaceId);
	}
}

void Joystick::update(int l_hor, int l_ver, int r_hor, int r_ver, int camera, bool buttonLB, bool buttonRB, bool buttonHome)
{
	l_hor -= 364;
	l_ver -= 364;
	r_hor -= 364;
	r_ver -= 364;
	camera -= 364;

	l_hor *= 4096;
	l_ver *= 4096;
	r_hor *= 4096;
	r_ver *= 4096;
	camera *= 4096;

	l_hor /= 165;
	l_ver /= 165;
	r_hor /= 165;
	r_ver /= 165;
	camera /= 165;

	this->thrustSpeed = l_ver;
	this->cameraSpeed = camera;

	if (!this->lastButtonHome &&  buttonHome)
	{
		if (this->homeValue == 0) this->homeValue = 16384;
		else if (this->homeValue == 16384) this->homeValue = 32768;
		else this->homeValue = 0;

		this->switchHome ^= true;
	}
	this->lastButtonHome = buttonHome;

	if (!this->lastButtonLB && buttonLB)
	{
		this->switchLB ^= true;
	}
	this->lastButtonLB = buttonLB;

	if (!this->lastButtonRB && buttonRB)
	{
		this->switchRB ^= true;
	}
	this->lastButtonRB = buttonRB;

	// center all sticks for users who fly with no springs at all (for calibration or to scratch your nose)
	// set camera to max (right) and flip left and right switches down fully to center sticks data.

	int range_max = 32000;
	int center_sticks = range_max / 2;

	button_1 = buttonLB;
	button_2 = buttonRB;
	button_3 = buttonHome;

	if (log)
		printf("\n  mDjiController : \n\n  L vert: %-5d | L hori: %-5d | R vert: %-5d | R hori: %-5d | camera: %-5d \n  btn1: %-d | btn2: %-d | btn3: %-d \n", l_ver, l_hor, r_ver, r_hor, camera, button_1, button_2, button_3 );

	// Send stick values to vJoy
	SetAxis(l_hor, interfaceId, HID_USAGE_X);
	SetAxis(l_ver, interfaceId, HID_USAGE_Y);
	SetAxis(r_hor, interfaceId, HID_USAGE_RX);
	SetAxis(r_ver, interfaceId, HID_USAGE_RY);
	SetAxis(camera, interfaceId, HID_USAGE_Z);
	SetAxis(this->homeValue, interfaceId, HID_USAGE_RZ);

	// Send button values to vJoy
	SetBtn(button_1, interfaceId, 1);
	SetBtn(button_2, interfaceId, 2);
	SetBtn(button_3, interfaceId, 3);
	SetBtn(this->switchLB, interfaceId, 4);
	SetBtn(this->switchRB, interfaceId, 5);
	SetBtn(this->switchHome, interfaceId, 6);
}

void Joystick::tick()
{
	this->thrustValue += (this->thrustSpeed -16384)/ 50;
	if (this->thrustValue < 0) this->thrustValue = 0;
	if (this->thrustValue > 32768) this->thrustValue = 32768;
	SetAxis(this->thrustValue, interfaceId, HID_USAGE_SL0);

	this->cameraValue += (this->cameraSpeed - 16384) / 50;
	if (this->cameraValue < 0) this->cameraValue = 0;
	if (this->cameraValue > 32768) this->cameraValue = 32768;
	SetAxis(this->cameraValue, interfaceId, HID_USAGE_SL1);
}


Joystick::~Joystick()
{
	RelinquishVJD(interfaceId);
	printf("  Disconnected from joystick\n");
}

