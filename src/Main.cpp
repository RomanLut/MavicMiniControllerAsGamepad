
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "Serial.h"
#include "Joystick.h"



short littleEndiansToShort(int first, int second) {
	if (first < 0) {
		first = 256 + first;
	}

	short combined = second << 8 | first;
	return combined;

}

void run(char* portName, int stickId, int logging) {

	char initData[] = { 0x55, 0xaa, 0x55, 0xaa, 0x1e, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x80, 0x00, 0x04, 0x04, 0x74, 0x94, 0x35, 0x00, 0xd8, 0xc0, 0x41, 0x00, 0x30, 0xf6, 0x08, 0x00, 0x00, 0xf6, 0x69, 0x9c, 0x01, 0xe8};
	char pingData[] = { 0x55, 0xaa, 0x55, 0xaa, 0x1e, 0x00, 0x01, 0x00, 0x00, 0x1c, 0x02, 0x00, 0x80, 0x00, 0x06, 0x01, 0x28, 0x97, 0xae, 0x03, 0x28, 0x36, 0xa4, 0x03, 0x28, 0x36, 0xa4, 0x03, 0xab, 0xa7, 0x30, 0x00, 0x03, 0x53};

	char incomingData[256] = "";
	int dataLength = 256;
	int readResult = 0;
	bool shouldRun = true;


	Joystick j(stickId, logging);
	if (!j.isConnected()) {
		printf("Couldn't connect to vJoy, quitting...\n");
		return;
	}
	printf("\n");

	Serial s(portName);
	if (!s.IsConnected()) {
		printf("Couldn't connect to COM port, quitting...\n");
		return;
	}

	printf("\nEverything is ready\n\n");
	s.WriteData(initData, 34);

	printf("Running...\nPress END key to quit\n");
	Sleep(2000);

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

			// update our virtual joystick
			j.update(left_horizontal, left_vertical, right_horizontal, right_vertical, left_lever, right_lever, camera);

		}


		if (GetAsyncKeyState(VK_END)) {
			shouldRun = false;
			printf("\n\nDetected END key, quitting...\n");
		}

		Sleep(10);
	}

}

int main() {
	std::string in;
	int portNr, stickId, logging;

	printf("\n  If confused, please read the readme file\n\n   NOTE: DJI_WIN Driver must be installed.\n   NOTE: DJI Assistant 2 Must be installed - Version 1.0.5 tested.\n   NOTE: Vjoy must be installed.\n\n  Device manager > View > Show Hidden > Ports COM & LPT\n  > DJI USB Virtual COM(COM#) <- THIS NUMBER!\n\n  Type port number here then press enter (default set to port 6): ");
	std::getline(std::cin, in);
	portNr = atoi(in.c_str());

	if (portNr < 1) {
		portNr = 6;
	}

	char port[100];
	sprintf_s(port, "COM%d", portNr);


	printf("\n\n  Select vJoy configuration (default set to 1): ");
	std::getline(std::cin, in);
	stickId = atoi(in.c_str());

	if (stickId < 1) {
		stickId = 1;
	}


	printf("\n\n  Disable logging? 1 for off 0 for on (default set to 0): ");
	std::getline(std::cin, in);
	logging = atoi(in.c_str());

	if (logging != 0) {
		logging = 1;
	}

	printf("  Starting...\n\n");
	run(port, stickId, logging);
	printf("  Closing down...\n\n");


	system("pause");
	return 0;
}