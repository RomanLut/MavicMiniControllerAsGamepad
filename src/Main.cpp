
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "StdAfx.h"
#include "Serial.h"
#include "Joystick.h"
#include "clearscreen.h"
#include "enumser.h"

// Check if settings.mdji exists
bool fileExists(const std::string& filename) {
	struct stat buf;
	return (stat(filename.c_str(), &buf) == 0);
}

// Convert Endians from controller
short littleEndiansToShort(int first, int second) {
	if (first < 0) {
		first = 256 + first;
	}

	short combined = second << 8 | first;
	return combined;

}

// Run the program loop for controller data
void run(char* portName, int stickId, int logging) {

	char initData[] = { 0x55, 0xaa, 0x55, 0xaa, 0x1e, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x80, 0x00, 0x04, 0x04, 0x74, 0x94, 0x35, 0x00, 0xd8, 0xc0, 0x41, 0x00, 0x30, 0xf6, 0x08, 0x00, 0x00, 0xf6, 0x69, 0x9c, 0x01, 0xe8};
	char pingData[] = { 0x55, 0xaa, 0x55, 0xaa, 0x1e, 0x00, 0x01, 0x00, 0x00, 0x1c, 0x02, 0x00, 0x80, 0x00, 0x06, 0x01, 0x28, 0x97, 0xae, 0x03, 0x28, 0x36, 0xa4, 0x03, 0x28, 0x36, 0xa4, 0x03, 0xab, 0xa7, 0x30, 0x00, 0x03, 0x53};

	char incomingData[256] = "";
	int dataLength = 256;
	int readResult = 0;
	bool shouldRun = true;


	Joystick j(stickId, logging);
	if (!j.isConnected()) {
		printf("  Couldn't connect to vJoy, quitting...\n");
		return;
	}
	printf("\n");

	Serial s(portName);
	if (!s.IsConnected()) {
		printf("  Couldn't connect to COM port, quitting...\n");
		return;
	}

	printf("\n  Everything is ready\n\n");
	s.WriteData(initData, 34);

	printf("  Running...\n  Press END key to quit\n");
	Sleep(2000);

	while (s.IsConnected() && shouldRun)
	{
		ClearScreen();

		s.WriteData(pingData, 34); // write this once in a while, otherwise it stops sending? :O
		readResult = s.ReadData(incomingData, dataLength);

		if (readResult == 76 && incomingData[0] == 0x55) { // probably positioning data
			short left_vertical = littleEndiansToShort(incomingData[39], incomingData[40]);
			short left_horizontal = littleEndiansToShort(incomingData[43], incomingData[44]);

			short right_horizontal = littleEndiansToShort(incomingData[31], incomingData[32]);
			short right_vertical = littleEndiansToShort(incomingData[35], incomingData[36]);

			short left_lever = littleEndiansToShort(incomingData[47], incomingData[48]);
			short right_lever = littleEndiansToShort(incomingData[51], incomingData[52]);

			short camera = littleEndiansToShort(incomingData[55], incomingData[56]);

			// update our virtual joystick
			j.update(left_horizontal, left_vertical, right_horizontal, right_vertical, left_lever, right_lever, camera);
			
		}

		if (GetAsyncKeyState(VK_END)) {
			shouldRun = false;
			printf("\n\n  Detected END key, quitting...\n");
		}

		Sleep(10);
	}

}


int main() {

	int portNr, stickId, logging;
	int data;
	logging = 0;
	std::string in;

	// Check if settings file exists, if so, open it and plug in the info.
	// If not, run the setup to create the settings file.
	if (fileExists ("settings.mdji")) {
		std::ifstream readSettings("settings.mdji");
		if (!readSettings) {
			printf("  Error Opening settings.mdji\n");
		}
		else {
			int i; i = 0;
			while (readSettings >> data) {

				i++;

				switch (i) {
				case 1: portNr = data;
					break;
				case 2: stickId = data;
					break;
				case 3: logging = data;
				}
			}
		}

		readSettings.close();

		char port[100];
		sprintf_s(port, "COM%d", portNr);

		printf("\n  Starting...\n\n");
		run(port, stickId, logging);
		printf("\n  Closing down...\n\n");

	}
	else {

		// No file exists, so go ahead and create one now.
		printf("\n  !!! If you input incorrect data, delete the settings.mdji file. !!!\n\n   NOTE: DJI_WIN Driver must be installed.\n   NOTE: DJI Assistant 2 Must be installed - Version 1.0.5 tested.\n   NOTE: vJoy must be installed.\n\n  After the above is installed correctly,\n  Turn on your connected DJI Phantom 2 Controller.\n  Once your controller is on and connected, press enter.\n\n  ");

		system("pause");

		// Clear screen to begin settings process
		ClearScreen();


		// BEGIN ENUMSER PORT SCAN

		//Initialize COM (Required by CEnumerateSerial::UsingWMI)
		HRESULT hr = CoInitialize(nullptr);
		if (FAILED(hr))
		{
			_tprintf(_T("  Failed to initialize COM, Error:%x\n"), hr);
			return hr;
		}

		//Initialize COM security (Required by CEnumerateSerial::UsingWMI)
		hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
		if (FAILED(hr))
		{
			_tprintf(_T("  Failed to initialize COM security, Error:%08X\n"), hr);
			CoUninitialize();
			return hr;
		}

		CEnumerateSerial::CPortsArray ports;
		CEnumerateSerial::CPortAndNamesArray portAndNames;
		CEnumerateSerial::CNamesArray names;

		#ifndef NO_CENUMERATESERIAL_USING_WMI
		_tprintf(_T("\n  The following COM devices are currently connected to your PC."));
		hr = CEnumerateSerial::UsingWMI(portAndNames);
		if (SUCCEEDED(hr))
		{
			int i = 0;

			printf("\n\n\n  ---------- BEGIN CONNECTED DEVICES ----------\n\n\n");
			for (const auto& port : portAndNames) {
				_tprintf(_T("  > COM%u <%s>"), port.first, port.second.c_str());
			}
			printf("\n\n\n  ---------- END  CONNECTED  DEVICES ----------\n\n\n");
		}
		else
			_tprintf(_T("  mDjiController was not able to scan your ports\n\n  Device manager > View > Show Hidden > Ports COM & LPT\n  > DJI USB Virtual COM(COM#) <- THIS NUMBER!"), hr);
		#endif //#ifndef NO_CENUMERATESERIAL_USING_WMI

		CoUninitialize();
		// END ENUMSER PORT SCAN


		// Start user input for settings file.
		printf("  What is your DJI Phantom Controller (COM#) number? : ");

		std::getline(std::cin, in);
		portNr = atoi(in.c_str());

		if (portNr < 1) {
			portNr = 1;
		}

		char port[100];
		sprintf_s(port, "COM%d", portNr);

		printf("\n\n  Set your vJoy virtual controller number (default set to 1): ");
		std::getline(std::cin, in);
		stickId = atoi(in.c_str());

		if (stickId < 1) {
			stickId = 1;
		}

		// Create a file to store settings in
		std::ofstream writeSettings("settings.mdji");

		if (!writeSettings) {
			printf("  Error Opening settings.mdji\n\n");
		}
		else {
			writeSettings << portNr << std::endl << stickId << std::endl << logging << std::endl;
			writeSettings.close();

			printf("\n  Starting...\n\n");
			run(port, stickId, logging);
			printf("\n  Closing down...\n\n");
		}
	}

	printf("  ");
	system("pause");
	return 0;
}