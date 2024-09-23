#pragma once

#include <iostream>
#include <windows.h>
#include <vector>

#include "instruction.h"
#include "virtual_instruction.h"

class Function
{
public:
	Function(DWORD startRva, DWORD endRva, std::vector<BYTE> bytes);

	bool disassemble();
	bool compileInstructionsToVirtualInstructions();
	void resolveBranchInstructions(DWORD bytecodeRva);
	DWORD getBytecodeIndex(int instructionIndex);

	std::vector<BYTE> getVirtualInstructionBytes();
	DWORD getStartRva();
	DWORD getEndRva();
private:
	DWORD startRva;
	DWORD endRva;
	std::vector<BYTE> bytes;
	std::vector<Instruction> instructions;
};