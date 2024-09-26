#pragma once

#include <windows.h>
#include <vector>
#include <random>

#include "vm.h"
#include "vm_handler.h"
#include "util.h"

class VMSection
{
public:
	void initialise(DWORD virtualAddress, DWORD pointerToRawData);
	bool isInitialised();
	DWORD getWritePointer();
	DWORD getWritePointerFileOffset();
	DWORD getWritePointerRva();
	std::vector<BYTE> getBytes();
	void addVmTramp(DWORD bytecodeRva);
	void addBytes(std::vector<BYTE> bytes);
private:
	bool initialised = false;
	DWORD pointerToRawData;
	DWORD virtualAddress;
	DWORD writePointer;
	std::vector<BYTE> bytes;

	void addVmHandlers();
	void addVmHandler(VMHandler vmHandler);
};