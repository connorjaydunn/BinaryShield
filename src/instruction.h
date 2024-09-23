#pragma once

#include <iostream>
#include <windows.h>
#include <vector>
#include <Zydis/Zydis.h>

#include "vm.h"
#include "virtual_instruction.h"

class Instruction
{
public:
	Instruction(ZydisDecodedInstruction instructionInfo, ZydisDecodedOperand* operandInfo, DWORD rva);

	bool compileToVirtualInstructions();
	void addVirtualInstruction(VirtualInstruction virtualInstruction);
	bool isBranchInstruction();
	bool isConditionalBranchInstruction();
	void setDestInstructionIndex(int destInstructionIndex);
	DWORD getRva();
	ZydisDecodedInstruction getInstructionInfo();
	std::vector<ZydisDecodedOperand> getOperandInfo();
	int getDestInstructionIndex();
	std::vector<BYTE> getVirtualInstructionBytes();
	std::vector<VirtualInstruction>& getVirtualInstructions();
private:
	DWORD rva;
	int destInstructionIndex; // index of instruction the current instruction will jump or call
	ZydisDecodedInstruction instructionInfo;
	std::vector<ZydisDecodedOperand> operandInfo;
	std::vector<VirtualInstruction> virtualInstructions;
};