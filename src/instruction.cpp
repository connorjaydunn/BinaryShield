#include "instruction.h"

Instruction::Instruction(ZydisDecodedInstruction instructionInfo, ZydisDecodedOperand* operandInfo, DWORD rva) : instructionInfo(instructionInfo), rva(rva)
{
	this->operandInfo = std::vector<ZydisDecodedOperand>(operandInfo, operandInfo + instructionInfo.operand_count);
}

bool Instruction::compileToVirtualInstructions()
{
	// generate the corrosponding virtual instructions
	if (!VM::compileInstructionToVirtualInstructions(this))
	{
		std::cerr << "error compiling instruction" << std::endl;
		return 0;
	}

	return 1;
}

void Instruction::addVirtualInstruction(VirtualInstruction virtualInstruction) { virtualInstructions.push_back(virtualInstruction); }

bool Instruction::isBranchInstruction()
{
	switch (instructionInfo.mnemonic)
	{
	case ZYDIS_MNEMONIC_JNBE:
	case ZYDIS_MNEMONIC_JB:
	case ZYDIS_MNEMONIC_JBE:
	case ZYDIS_MNEMONIC_JCXZ:
	case ZYDIS_MNEMONIC_JECXZ:
	case ZYDIS_MNEMONIC_JKNZD:
	case ZYDIS_MNEMONIC_JKZD:
	case ZYDIS_MNEMONIC_JL:
	case ZYDIS_MNEMONIC_JLE:
	case ZYDIS_MNEMONIC_JNB:
	case ZYDIS_MNEMONIC_JNL:
	case ZYDIS_MNEMONIC_JNLE:
	case ZYDIS_MNEMONIC_JNO:
	case ZYDIS_MNEMONIC_JNP:
	case ZYDIS_MNEMONIC_JNS:
	case ZYDIS_MNEMONIC_JNZ:
	case ZYDIS_MNEMONIC_JO:
	case ZYDIS_MNEMONIC_JP:
	case ZYDIS_MNEMONIC_JRCXZ:
	case ZYDIS_MNEMONIC_JS:
	case ZYDIS_MNEMONIC_JZ:
	case ZYDIS_MNEMONIC_JMP:
		return 1;
	}
	return 0;
}

// if instruction is a branch instruction, but not JMP, then it must be a conditional branch
bool Instruction::isConditionalBranchInstruction() { return (isBranchInstruction() && instructionInfo.mnemonic != ZYDIS_MNEMONIC_JMP); }

void Instruction::setDestInstructionIndex(int destInstructionIndex) { this->destInstructionIndex = destInstructionIndex; }

DWORD Instruction::getRva() { return rva; }

ZydisDecodedInstruction Instruction::getInstructionInfo() { return instructionInfo; }

std::vector<ZydisDecodedOperand> Instruction::getOperandInfo() { return operandInfo; }

int Instruction::getDestInstructionIndex() { return destInstructionIndex; }

std::vector<BYTE> Instruction::getVirtualInstructionBytes()
{
	std::vector<BYTE> bytes;

	// iterate over each virtual instruction and get its raw bytes
	for (int i = 0; i < virtualInstructions.size(); i++)
	{
		std::vector<BYTE> virtualInstructionBytes = virtualInstructions[i].getBytes();
		bytes.insert(bytes.end(), virtualInstructionBytes.begin(), virtualInstructionBytes.end());
	}

	return bytes;
}

std::vector<VirtualInstruction>& Instruction::getVirtualInstructions() { return virtualInstructions; }