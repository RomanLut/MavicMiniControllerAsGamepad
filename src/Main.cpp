
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

	//unsigned char pingData[] = { 0x55, 0x0d, 0x04, 0x33, 0x0a, 0x0e, 0x04, 0x00, 0x40, 0x06, 0x27, 0x1c, 0x3e };
	//unsigned char pingData[] = { 0x55, 0x0d, 0x04, 0x33, 0x0a, 0x0e, 0x05, 0x00, 0x40, 0x06, 0x01, 0x6c, 0x71 };

	unsigned char pingData[] = { 0x55, 0x0d, 0x04, 0x33, 0x0a, 0x0e, 0x03, 0x00, 0x40, 0x06, 0x01, 0xf4, 0x4a };
	int pingDataLen = 13;

	unsigned char pingData2[] = { 0x55, 0x0d, 0x04, 0x33, 0x0a, 0x0e, 0x02, 0x00, 0x40, 0x06, 0x27, 0x84, 0x05 };
	int pingData2Len = 13;

	unsigned char incomingData[256] = "";
	int bufferLength = 256;
	int dataLength = 0;
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

	printf("  Running...\n\n  Press Ctrl+C to quit\n\n");
	Sleep(1000);

	bool buttonLB = false;
	bool buttonRB = false;
	bool buttonHome = false;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	int timeoutCount = 0;
	int timeoutCount2 = 0;

	int errorCount = 0;

	while (s.IsConnected() && shouldRun)
	{
		timeoutCount++;
		if (timeoutCount == 2 )
		{
			timeoutCount = 0;
			if (s.WriteData(pingData, pingDataLen))
			{
				errorCount = 0;
			}
			else
			{
				errorCount++;
				if (errorCount == 200)
				{
					printf("\n\n  Device disconnected!\n\n");
					return;
				}
			}
		}
		
		timeoutCount2++;
		if (timeoutCount2 == 5)
		{
			s.WriteData(pingData2, pingData2Len);
			timeoutCount2 = 0;
		}

		Sleep(5);
		readResult = s.ReadData(&incomingData[dataLength], bufferLength - dataLength);
		dataLength += readResult;

		if (logging && readResult > 0 )
		{
			printf("\n\n");
			for (int i = 0; i < dataLength; i++)
			{
				printf("%02X ", (i & 0xff));
			}
			printf("\n");
			for (int i = 0; i < dataLength; i++)
			{
				printf("%02X ", incomingData[i]);
			}
			printf("\n\n");
		}

		//discard any trash and unknown packets
		if (dataLength >= 2 && incomingData[0] == 0x55 && incomingData[1] != 0x26 && incomingData[1] != 0x3a) {
			incomingData[0] = 0;
		}
		while (dataLength > 0 && incomingData[0] != 0x55)
		{
			dataLength--;
			memcpy(incomingData, incomingData + 1, dataLength);
		}
		
		//pingData2 responce
		//it contains uncalibrated Axis values and button flags
		if (dataLength >= 0x3a  && incomingData[0] == 0x55 && incomingData[1] == 0x3A ) {
			buttonLB = (incomingData[0x1d] & 0x40) != 0;
			buttonRB = (incomingData[0x1d] & 0x20) != 0;
			buttonHome = (incomingData[0x1d] & 0x80) != 0;

			dataLength -= 0x3a;
			memcpy(incomingData, incomingData + 0x3a, dataLength);
		}

		//pingdata response
		//contains calibrated axis values, no button flags
		if (dataLength >=0x26  && incomingData[0] == 0x55 && incomingData[1] == 0x26) {
			short left_vertical = littleEndiansToShort(incomingData[19], incomingData[20]);
			short left_horizontal = littleEndiansToShort(incomingData[22], incomingData[23]);

			short right_horizontal = littleEndiansToShort(incomingData[13], incomingData[14]);
			short right_vertical = littleEndiansToShort(incomingData[16], incomingData[17]);

			short camera = littleEndiansToShort(incomingData[25], incomingData[26]);

			dataLength -= 0x26;
			memcpy(incomingData, incomingData + 0x26, dataLength);

			if (logging)
			{
				ClearScreen();
			}

			// update our virtual joystick
			j.update(left_horizontal, left_vertical, right_horizontal, right_vertical, camera, buttonLB, buttonRB, buttonHome);
		}

		/*
		if (GetAsyncKeyState(VK_END)) {
			shouldRun = false;
			printf("\n\n  Detected END key, quitting...\n");
		}
		*/

		j.tick();
	}

}


int main() {

	int portNr = 0, stickId, logging;
	int data;
	logging = 0;
	std::string in;
	std::string saveSettings = "o";

	ClearScreen();
/*
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 20;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;

    wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Lucida Console");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
*/

	printf("    ________                                              .___\n");
	printf("   /  _____/ _____     _____    ____  ______  _____     __| _/\n");
	printf("  /   \\  ___ \\__  \\   /     \\ _/ __ \\ \\____ \\ \\__  \\   / __ | \n");
	printf("  \\    \\_\\  \\ / __ \\_|  Y Y  \\\\  ___/ |  |_> > / __ \\_/ /_/ | \n");
	printf("   \\______  /(____  /|__|_|  / \\___  >|   __/ (____  /\\____ |\n");
	printf("          \\/      \\/       \\/      \\/ |__|         \\/      \\/ \  v1.0\n");
	printf("\n");


	// Check if settings file exists, if so, open it and plug in the info.
	// If not, run the setup to create the settings file.
	/*
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
	else*/ {

		/*
		// No file exists, so user can input data.
		printf("\n  !!! If you input incorrect data, delete the settings.mdji file. !!!\n\n   NOTE: DJI_WIN Driver must be installed.\n   NOTE: DJI Assistant 2 Must be installed - Version 1.0.5 tested.\n   NOTE: vJoy must be installed.\n\n   !IMPORTANT! - DJI Assistant cannot be running at the same time\n   as this program or it will not connect.\n\n  After the above is installed correctly,\n  Turn on your connected DJI Phantom 2 Controller.\n  Once your controller is on and connected, press enter.\n\n");

		printf("  ");
		system("pause");

		// Clear screen to begin settings process
		ClearScreen();

		*/
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

		//std::cout << "\n  ---------- CONNECTED DEVICES ----------\n\n" << std::endl;

		std::cout << "\n  Searching for connected DJI Mavic Mini 1 controllers..." << std::endl;

		#ifndef NO_CENUMERATESERIAL_USING_WMI
		hr = CEnumerateSerial::UsingWMI(portAndNames);
		if (SUCCEEDED(hr))
		{

			for (const auto& port : portAndNames) {
				std::string parseCom = port.second;
				std::size_t found = parseCom.find("DJI");
					if (found != std::string::npos) {
						std::cout << "  =>" << port.second << std::endl;
						portNr = port.first;
					}
			}
		}
		else
			_tprintf(_T("  ERROR scanning ports!"), hr);
		#endif //#ifndef NO_CENUMERATESERIAL_USING_WMI

		CoUninitialize();

		/*
		if (portNr == 0) {
			std::cout << "  No DJI devices found!" << std::endl;
		}
		*/

		//std::cout << "\n\n  ---------------------------------------\n\n" << std::endl;

		// Start user input for settings.
		if (portNr != 0) {
			std::cout << "  Found controller at COM" << portNr << " :)\n" << std::endl;
			printf("  ");
			//system("pause"); 

		}
		else {

			std::cout << "  No Mavic Mini 1 controllers found:(\n\n" << std::endl;

			printf("  ");
			system("pause");

			return 0;

		}

		//ClearScreen();

		// User sets the vJoy controller number
		//printf("\n\n  What vJoy controller should we attach to? (default set to 1): ");
		//std::getline(std::cin, in);
		//stickId = atoi(in.c_str());
		stickId = 1;

		if (stickId < 1) {
			stickId = 1;
		}

		char port[100];
		sprintf_s(port, "COM%d", portNr);

		/*
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
		*/

		//ShowWindow(GetConsoleWindow(), SW_SHOW);

		run(port, stickId, logging);

	}

	printf("  ");
	system("pause");
	return 0;
}