#pragma once 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/*
* Adopted from http://playground.arduino.cc/Interfacing/CPPWindows
*/

class Serial
{
private:
	HANDLE hSerial;
	bool connected;
	COMSTAT status;
	DWORD errors;

public:
	Serial(char *portName);
	~Serial();
	int ReadData(unsigned char *buffer, unsigned int nbChar);
	bool WriteData(unsigned char *buffer, unsigned int nbChar);
	bool IsConnected();
};
