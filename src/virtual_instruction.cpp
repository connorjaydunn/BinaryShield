#include "virtual_instruction.h"

VirtualInstruction::VirtualInstruction(DWORD vmHandlerRva, long long operand, BYTE operandSize) 
	: vmHandlerRva(vmHandlerRva),
	operand(operand),
	operandSize(operandSize/8),
	hasOperand(true)
{};

VirtualInstruction::VirtualInstruction(DWORD vmHandlerRva) : vmHandlerRva(vmHandlerRva) {};

std::vector<BYTE> VirtualInstruction::getBytes()
{
	std::vector<BYTE> bytes;
	
	// convert vmHandlerRva into a BYTE vector
	std::vector<BYTE> vmHandlerRvaBytes = convertToByteVector<DWORD>(vmHandlerRva);
	bytes.insert(bytes.end(), vmHandlerRvaBytes.begin(), vmHandlerRvaBytes.end());

	// if operand exists, convert it into a BYTE vector
	if (hasOperand)
	{
		std::vector<BYTE> operandBytes;

		switch (operandSize)
		{
		case 8: operandBytes = convertToByteVector<long long>(operand); break;
		case 4: operandBytes = convertToByteVector<DWORD>(operand); break;
		case 2: operandBytes = convertToByteVector<WORD>(operand); break;
		case 1: operandBytes = convertToByteVector<BYTE>(operand); break;
		}

		bytes.insert(bytes.end(), operandBytes.begin(), operandBytes.end());
	}

	return bytes;
}

void VirtualInstruction::setOperand(long long operand) { this->operand = operand; }