#pragma once

#include <windows.h>
#include <vector>

#include "util.h"

class VirtualInstruction
{
public:
	VirtualInstruction(DWORD vmHandlerRva, long long operand, BYTE operandSize);
	VirtualInstruction(DWORD vmHandlerRva);

	std::vector<BYTE> getBytes();

	void setOperand(long long operand);
private:
	DWORD vmHandlerRva;
	long long operand;
	bool hasOperand = false;
	BYTE operandSize;
};