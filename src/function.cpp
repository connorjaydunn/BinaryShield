#include "function.h"

Function::Function(DWORD startRva, DWORD endRva, std::vector<BYTE> bytes) : startRva(startRva), endRva(endRva), bytes(bytes) {};

bool Function::disassemble()
{
	// initialise zydis decoder
	ZydisDecoder decoder;
	if (ZYAN_FAILED(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)))
	{
		std::cerr << "error initializing ZydisDecoder" << std::endl;
		return 0;
	}

	ZydisDecodedInstruction instructionInfo;
	ZydisDecodedOperand operandInfo[ZYDIS_MAX_OPERAND_COUNT];

	// first pass: disassemble each instruction
	DWORD offset = 0;
	while (offset < bytes.size())
	{
		if (ZYAN_FAILED(ZydisDecoderDecodeFull(&decoder, bytes.data() + offset, bytes.size(), &instructionInfo, operandInfo)))
		{
			std::cerr << "error decoding instruction" << std::endl;
			return 0;
		}

		instructions.push_back(Instruction(instructionInfo, operandInfo, offset + startRva));

		offset += instructionInfo.length;
	}

	// second pass: find branch destinations
	for (int i = 0; i < instructions.size(); i++)
	{
		if (instructions[i].isBranchInstruction())
		{
			for (int j = 0; j < instructions.size(); j++)
			{
				// check if instruction is destination of our branch
				if (instructions[i].getRva() +
					instructions[i].getOperandInfo()[0].imm.value.s +
					instructions[i].getInstructionInfo().length == instructions[j].getRva())
				{
					instructions[i].setDestInstructionIndex(j);
				}
			}
		}
	}

	return 1;
}

bool Function::compileInstructionsToVirtualInstructions()
{
	// push context onto virtual stack
	VM::popVmContext(&instructions[0]);
	instructions[0].compileToVirtualInstructions();

	// compile each instruction to a set of virtual instructions
	for (int i = 1; i < instructions.size(); i++)
	{
		if (!instructions[i].compileToVirtualInstructions())
			// unable to virtualise instruciton, likely no support implemented
			return 0;
	}

	return 1;
}

void Function::resolveBranchInstructions(DWORD bytecodeRva)
{
	for (int i = 0; i < instructions.size(); i++)
	{
		if (instructions[i].isBranchInstruction())
		{
			// get bytecode rva of the destination instruction
			DWORD targetBytecodeRva = getBytecodeIndex(instructions[i].getDestInstructionIndex()) + bytecodeRva;

			// set branch instructions operand to rva of destination instruction's bytecode rva
			if (instructions[i].isConditionalBranchInstruction())
			{
				// if conditional branch, second virtual instruction is the "push target.rva"
				instructions[i].getVirtualInstructions()[1].setOperand(targetBytecodeRva);
			}
			else
			{
				// if non-conditional branch, first virtual instruction is the "push target.rva"
				instructions[i].getVirtualInstructions()[0].setOperand(targetBytecodeRva);
			}
		}
	}
}

DWORD Function::getBytecodeIndex(int instructionIndex)
{
	DWORD bytecodeIndex = 0;

	/*
		the following is a hacky solution to the fact instruction[0] will have "non-real" instructions
		to deal with the vm context. A better solution will be required in order to support x86 call.
		Perhaps we should implement a second vector of type VirtualInstruction that will store these
		"injected" instructions.
	*/
	if (instructionIndex == 0)
	{
		// length of "injected" virtual instructions (VM::popVmContext())
		bytecodeIndex = 0x55;
	}

	// calculate the bytecode index of instructionIndex via summing all previous instruction bytecode sizes
	for (int i = 0; i < instructionIndex; i++)
		bytecodeIndex += instructions[i].getVirtualInstructionBytes().size();

	return bytecodeIndex;
}

std::vector<BYTE> Function::getVirtualInstructionBytes()
{
	std::vector<BYTE> bytes;

	for (int i = 0; i < instructions.size(); i++)
	{
		std::vector<BYTE> virtualInstructionBytes = instructions[i].getVirtualInstructionBytes();
		bytes.insert(bytes.end(), virtualInstructionBytes.begin(), virtualInstructionBytes.end());
	}

	return bytes;
}

DWORD Function::getStartRva() { return startRva; }

DWORD Function::getEndRva() { return endRva; }