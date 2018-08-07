
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

	printf("  Running...\n\n  Press END key to quit\n\n");
	Sleep(4000);

	while (s.IsConnected() && shouldRun)
	{

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

			ClearScreen();

			// update our virtual joystick
			j.update(left_horizontal, left_vertical, right_horizontal, right_vertical, left_lever, right_lever, camera);
			
		}

		if (GetAsyncKeyState(VK_END)) {
			shouldRun = false;
			printf("\n\n  Detected END key, quitting...\n");
		}


		Sleep(20);
	}

}


int main() {

	int portNr = 0, stickId, logging;
	int data;
	logging = 0;
	std::string in;
	std::string saveSettings = "o";

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

		// No file exists, so user can input data.
		printf("\n  !!! If you input incorrect data, delete the settings.mdji file. !!!\n\n   NOTE: DJI_WIN Driver must be installed.\n   NOTE: DJI Assistant 2 Must be installed - Version 1.0.5 tested.\n   NOTE: vJoy must be installed.\n\n   !IMPORTANT! - DJI Assistant cannot be running at the same time\n   as this program or it will not connect.\n\n  After the above is installed correctly,\n  Turn on your connected DJI Phantom 2 Controller.\n  Once your controller is on and connected, press enter.\n\n");

		printf("  ");
		system("pause");

		// Clear screen to begin settings process
		ClearScreen();


		/* Begin Enumser for auto device discovery.
		Initialize COM (Required by CEnumerateSerial::UsingWMI) */
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

		std::cout << "\n  ---------- CONNECTED DEVICES ----------\n\n" << std::endl;

		#ifndef NO_CENUMERATESERIAL_USING_WMI
		hr = CEnumerateSerial::UsingWMI(portAndNames);
		if (SUCCEEDED(hr))
		{

			for (const auto& port : portAndNames) {
				std::string parseCom = port.second;
				std::size_t found = parseCom.find("DJI");
					if (found != std::string::npos) {
						std::cout << "  " << port.second << std::endl;
						portNr = port.first;
					}
			}
		}
		else
			_tprintf(_T("  ERROR scanning ports!"), hr);
		#endif //#ifndef NO_CENUMERATESERIAL_USING_WMI

		CoUninitialize();

		if (portNr == 0) {
			std::cout << "  No DJI devices found!" << std::endl;
		}

		std::cout << "\n\n  ---------------------------------------\n\n" << std::endl;



		// Start user input for settings.
		if (portNr != 0) {
			std::cout << "  We found a DJI device attached at COM" << portNr << "\n\n" << std::endl;
			printf("  ");
			system("pause"); 

		}
		else {

			std::cout << "  No DJI devices attached, please make sure\n  that you have the following software pre-installed\n\n   > DJI WIN driver\n   > DJI Assistant 2 (Version 1.0.5 tested)\n\n" << std::endl;

			printf("  ");
			system("pause");

			return 0;

		}

		ClearScreen();

		// User sets the vJoy controller number
		printf("\n\n  What vJoy controller should we attach to? (default set to 1): ");
		std::getline(std::cin, in);
		stickId = atoi(in.c_str());

		if (stickId < 1) {
			stickId = 1;
		}

		char port[100];
		sprintf_s(port, "COM%d", portNr);

		// Create a file to store settings in if they want.
		while (saveSettings != "y" || saveSettings != "n") {

			std::cout << "\n\n  Save settings for auto start next time? ( y or n ) : ";
			std::getline(std::cin, saveSettings);

			if (saveSettings == "y") {

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
			else if (saveSettings == "n") {

				printf("\n  Starting...\n\n");
				run(port, stickId, logging);
				printf("\n  Closing down...\n\n");

			}

			ClearScreen();

		}
	}

	printf("  ");
	system("pause");
	return 0;
}