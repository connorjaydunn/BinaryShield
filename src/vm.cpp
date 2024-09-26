#include "instruction.h"
#include "vm.h"

namespace VM
{
	std::vector<VMHandler> vmHandlers;

	bool compileInstructionToVirtualInstructions(Instruction* instruction)
	{
		switch (instruction->getInstructionInfo().mnemonic)
		{
		case ZYDIS_MNEMONIC_PUSH: return x86PushHandler(instruction);
		case ZYDIS_MNEMONIC_POP: return x86PopHandler(instruction);
		case ZYDIS_MNEMONIC_MOV: return x86MovHandler(instruction);
		case ZYDIS_MNEMONIC_LEA: return x86LeaHandler(instruction);
		case ZYDIS_MNEMONIC_RET: return x86RetHandler(instruction);
		case ZYDIS_MNEMONIC_CMP: return x86CmpHandler(instruction);
		case ZYDIS_MNEMONIC_TEST: return x86TestHandler(instruction);
		case ZYDIS_MNEMONIC_JMP: return x86JmpHandler(instruction);
		case ZYDIS_MNEMONIC_JNZ: return x86JneHandler(instruction);
		case ZYDIS_MNEMONIC_NOP: return 1;

		case ZYDIS_MNEMONIC_ADD:
		case ZYDIS_MNEMONIC_SUB:
		case ZYDIS_MNEMONIC_XOR:
		case ZYDIS_MNEMONIC_AND:
		case ZYDIS_MNEMONIC_OR:
		case ZYDIS_MNEMONIC_SHL:
			return x86ArithmeticHandler(instruction, instruction->getInstructionInfo().mnemonic, true);
		}
		return 0;
	}

	bool x86PushHandler(Instruction* instruction)
	{
		std::vector<ZydisDecodedOperand> operandInfo = instruction->getOperandInfo();

		switch (operandInfo[0].type)
		{
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			emitPushRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
			return 1;
		}
		case ZYDIS_OPERAND_TYPE_IMMEDIATE:
		{
			emitPushImmediate(instruction, operandInfo[0].imm.value.s, operandInfo[0].size);
			return 1;
		}
		case ZYDIS_OPERAND_TYPE_MEMORY:
		{
			calculateEffectiveAddress(instruction, operandInfo[0].mem);
			emitRead(instruction, operandInfo[0].size);
			return 1;
		}
		case ZYDIS_OPERAND_TYPE_POINTER: return 0;
		case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
		}

		return 0;
	}

	bool x86PopHandler(Instruction* instruction)
	{
		std::vector<ZydisDecodedOperand> operandInfo = instruction->getOperandInfo();

		switch (operandInfo[0].type)
		{
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
			return 1;
		}
		case ZYDIS_OPERAND_TYPE_IMMEDIATE: return 0;
		case ZYDIS_OPERAND_TYPE_MEMORY: return 0;
		case ZYDIS_OPERAND_TYPE_POINTER: return 0;
		case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
		}
		
		return 0;
	}

	bool x86MovHandler(Instruction* instruction)
	{
		std::vector<ZydisDecodedOperand> operandInfo = instruction->getOperandInfo();

		switch (operandInfo[0].type)
		{
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			switch (operandInfo[1].type)
			{
			case ZYDIS_OPERAND_TYPE_REGISTER:
			{
				if (operandInfo[0].size == 32)
					zeroRegister(instruction, operandInfo[0].reg.value);
				emitPushRegister(instruction, operandInfo[1].reg.value, operandInfo[0].size);
				emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_IMMEDIATE:
			{
				if (operandInfo[0].size == 32)
					zeroRegister(instruction, operandInfo[0].reg.value);
				emitPushImmediate(instruction, operandInfo[1].imm.value.s, operandInfo[0].size);
				emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_MEMORY:
			{
				calculateEffectiveAddress(instruction, operandInfo[1].mem);
				if (operandInfo[0].size == 32)
					zeroRegister(instruction, operandInfo[0].reg.value);
				emitRead(instruction, operandInfo[0].size);
				emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_POINTER: return 0;
			case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
			}
		}
		case ZYDIS_OPERAND_TYPE_IMMEDIATE: return 0;
		case ZYDIS_OPERAND_TYPE_MEMORY:
		{
			calculateEffectiveAddress(instruction, operandInfo[0].mem);
			emitPopRegister(instruction, R0, 64);

			switch (operandInfo[1].type)
			{
			case ZYDIS_OPERAND_TYPE_REGISTER:
			{
				emitPushRegister(instruction, operandInfo[1].reg.value, operandInfo[0].size);
				emitPushRegister(instruction, R0, 64);
				emitWrite(instruction, operandInfo[0].size);
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_IMMEDIATE:
			{
				emitPushImmediate(instruction, operandInfo[1].imm.value.s, operandInfo[0].size);
				emitPushRegister(instruction, R0, 64);
				emitWrite(instruction, operandInfo[0].size);
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_MEMORY: return 0;
			case ZYDIS_OPERAND_TYPE_POINTER: return 0;
			case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
			}
		}
		case ZYDIS_OPERAND_TYPE_POINTER: return 0;
		case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
		}

		return 0;
	}

	bool x86LeaHandler(Instruction* instruction)
	{
		std::vector<ZydisDecodedOperand> operandInfo = instruction->getOperandInfo();

		calculateEffectiveAddress(instruction, operandInfo[1].mem);
		emitPopRegister(instruction, R0, 64);
		emitPushRegister(instruction, R0, operandInfo[0].size);
		if (operandInfo[0].size == 32)
			zeroRegister(instruction, operandInfo[0].reg.value);
		emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);

		return 1;
	}

	bool x86RetHandler(Instruction* instruction)
	{
		// add check for operand (i.e. ret 0xC)
		pushVmContext(instruction);
		emitExit(instruction);
		return 1;
	}

	bool x86CmpHandler(Instruction* instruction) { return x86ArithmeticHandler(instruction, ZYDIS_MNEMONIC_SUB, false); }

	bool x86TestHandler(Instruction* instruction) { return x86ArithmeticHandler(instruction, ZYDIS_MNEMONIC_AND, false); }

	bool x86JmpHandler(Instruction* instruction)
	{
		if (instruction->getOperandInfo()[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
			return 0;
		emitJmp(instruction);
		return 1;
	}

	bool x86JneHandler(Instruction* instruction)
	{
		if (instruction->getOperandInfo()[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
			return 0;
		emitPushRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
		emitJne(instruction);
		return 1;
	}

	bool x86ArithmeticHandler(Instruction* instruction, ZydisMnemonic operation, bool storeOutput)
	{
		std::vector<ZydisDecodedOperand> operandInfo = instruction->getOperandInfo();

		switch (operandInfo[0].type)
		{
		case ZYDIS_OPERAND_TYPE_REGISTER:
		{
			switch (operandInfo[1].type)
			{
			case ZYDIS_OPERAND_TYPE_REGISTER:
			{
				emitPushRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				emitPushRegister(instruction, operandInfo[1].reg.value, operandInfo[0].size);
				emitArithmetic(instruction, operation, operandInfo[0].size);
				emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
				if (storeOutput)
				{
					if (operandInfo[0].size == 32)
						zeroRegister(instruction, operandInfo[0].reg.value);
					emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				}
				else
				{
					emitPopRegister(instruction, R0, operandInfo[0].size);
				}
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_IMMEDIATE:
				emitPushRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				emitPushImmediate(instruction, operandInfo[1].imm.value.s, operandInfo[0].size);
				emitArithmetic(instruction, operation, operandInfo[0].size);
				emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
				if (storeOutput)
				{
					if (operandInfo[0].size == 32)
						zeroRegister(instruction, operandInfo[0].reg.value);
					emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				}
				else
				{
					emitPopRegister(instruction, R0, operandInfo[0].size);
				}
				return 1;
			case ZYDIS_OPERAND_TYPE_MEMORY:
			{
				calculateEffectiveAddress(instruction, operandInfo[1].mem);
				emitRead(instruction, operandInfo[0].size);
				emitPushRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				emitArithmetic(instruction, operation, operandInfo[0].size);
				emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);

				if (storeOutput)
				{
					if (operandInfo[0].size == 32)
						zeroRegister(instruction, operandInfo[0].reg.value);
					emitPopRegister(instruction, operandInfo[0].reg.value, operandInfo[0].size);
				}
				else
				{
					emitPopRegister(instruction, R0, operandInfo[0].size);
				}
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_POINTER: return 0;
			case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
			}
		}
		case ZYDIS_OPERAND_TYPE_IMMEDIATE:
			return 0;
		case ZYDIS_OPERAND_TYPE_MEMORY:
		{
			calculateEffectiveAddress(instruction, operandInfo[0].mem);
			emitPushRegister(instruction, ZYDIS_REGISTER_RSP, 64);
			emitRead(instruction, 64);
			emitPopRegister(instruction, R0, 64);

			switch (operandInfo[1].type)
			{
			case ZYDIS_OPERAND_TYPE_REGISTER:
			{
				emitRead(instruction, operandInfo[0].size);
				emitPushRegister(instruction, operandInfo[1].reg.value, operandInfo[0].size);
				emitArithmetic(instruction, operation, operandInfo[0].size);
				emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
				if (storeOutput)
				{
					emitPushRegister(instruction, R0, 64);
					emitWrite(instruction, operandInfo[0].size);
				}
				else
				{
					emitPopRegister(instruction, R0, operandInfo[0].size);
				}
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_IMMEDIATE:
			{
				emitRead(instruction, operandInfo[0].size);
				emitPushImmediate(instruction, operandInfo[1].imm.value.s, operandInfo[0].size);
				emitArithmetic(instruction, operation, operandInfo[0].size);
				emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
				if (storeOutput)
				{
					emitPushRegister(instruction, R0, 64);
					emitWrite(instruction, operandInfo[0].size);
				}
				else
				{
					emitPopRegister(instruction, R0, operandInfo[0].size);
				}
				return 1;
			}
			case ZYDIS_OPERAND_TYPE_MEMORY: return 0;
			case ZYDIS_OPERAND_TYPE_POINTER: return 0;
			case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
			}
		}
		case ZYDIS_OPERAND_TYPE_POINTER: return 0;
		case ZYDIS_OPERAND_TYPE_UNUSED: return 0;
		}

		return 0;
	}

	BYTE getVirtualRegisterIndex(ZydisRegister reg)
	{
		switch (reg)
		{
		case ZYDIS_REGISTER_RFLAGS:
			return 0;

		case ZYDIS_REGISTER_RAX:
		case ZYDIS_REGISTER_EAX:
		case ZYDIS_REGISTER_AX:
		case ZYDIS_REGISTER_AL:
			return 1;

		case ZYDIS_REGISTER_RBX:
		case ZYDIS_REGISTER_EBX:
		case ZYDIS_REGISTER_BX:
		case ZYDIS_REGISTER_BL:
			return 2;

		case ZYDIS_REGISTER_RCX:
		case ZYDIS_REGISTER_ECX:
		case ZYDIS_REGISTER_CX:
		case ZYDIS_REGISTER_CL:
			return 3;

		case ZYDIS_REGISTER_RDX:
		case ZYDIS_REGISTER_EDX:
		case ZYDIS_REGISTER_DX:
		case ZYDIS_REGISTER_DL:
			return 4;

		case ZYDIS_REGISTER_RSI:
		case ZYDIS_REGISTER_ESI:
		case ZYDIS_REGISTER_SI:
		case ZYDIS_REGISTER_SIL:
			return 5;

		case ZYDIS_REGISTER_RDI:
		case ZYDIS_REGISTER_EDI:
		case ZYDIS_REGISTER_DI:
		case ZYDIS_REGISTER_DIL:
			return 6;

		case ZYDIS_REGISTER_RBP:
		case ZYDIS_REGISTER_EBP:
		case ZYDIS_REGISTER_BP:
		case ZYDIS_REGISTER_BPL:
			return 7;

		case ZYDIS_REGISTER_R8:
		case ZYDIS_REGISTER_R8D:
		case ZYDIS_REGISTER_R8W:
		case ZYDIS_REGISTER_R8B:
			return 8;

		case ZYDIS_REGISTER_R9:
		case ZYDIS_REGISTER_R9D:
		case ZYDIS_REGISTER_R9W:
		case ZYDIS_REGISTER_R9B:
			return 9;

		case ZYDIS_REGISTER_R10:
		case ZYDIS_REGISTER_R10D:
		case ZYDIS_REGISTER_R10W:
		case ZYDIS_REGISTER_R10B:
			return 10;

		case ZYDIS_REGISTER_R11:
		case ZYDIS_REGISTER_R11D:
		case ZYDIS_REGISTER_R11W:
		case ZYDIS_REGISTER_R11B:
			return 11;

		case ZYDIS_REGISTER_R12:
		case ZYDIS_REGISTER_R12D:
		case ZYDIS_REGISTER_R12W:
		case ZYDIS_REGISTER_R12B:
			return 12;

		case ZYDIS_REGISTER_R13:
		case ZYDIS_REGISTER_R13D:
		case ZYDIS_REGISTER_R13W:
		case ZYDIS_REGISTER_R13B:
			return 13;

		case ZYDIS_REGISTER_R14:
		case ZYDIS_REGISTER_R14D:
		case ZYDIS_REGISTER_R14W:
		case ZYDIS_REGISTER_R14B:
			return 14;

		case ZYDIS_REGISTER_R15:
		case ZYDIS_REGISTER_R15D:
		case ZYDIS_REGISTER_R15W:
		case ZYDIS_REGISTER_R15B:
			return 15;

		default:
			return -1; // something went wrong
		}
	}

	BYTE getVirtualRegisterIndex(VIRTUAL_REGISTERS reg)
	{
		switch (reg)
		{
		case R0: return 16;
		case R1: return 17;

		default: return -1; // something went wrong
		}
	}

	DWORD getVmHandlerRva(VMHandlerTypes type)
	{
		// array of possible rvas we can return
		std::vector<DWORD> targetHandlersRva;

		for (int i = 0; i < vmHandlers.size(); i++)
		{
			if (vmHandlers[i].getType() == type)
			{
				targetHandlersRva.push_back(vmHandlers[i].getRva());
			}
		}

		// select a random handler from our array of target handlers
		return targetHandlersRva[getRandomInt(0, targetHandlersRva.size() - 1)];
	}

	void popVmContext(Instruction* instruction)
	{
		emitPopRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RAX, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RBX, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RCX, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RDX, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RSI, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RDI, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_RBP, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R8, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R9, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R10, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R11, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R12, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R13, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R14, 64);
		emitPopRegister(instruction, ZYDIS_REGISTER_R15, 64);
		emitPopRegister(instruction, R0, 64);
	}

	void pushVmContext(Instruction* instruction)
	{
		emitPushRegister(instruction, ZYDIS_REGISTER_R15, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R14, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R13, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R12, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R11, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R10, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R9, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_R8, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RBP, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RDI, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RSI, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RDX, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RCX, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RBX, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RAX, 64);
		emitPushRegister(instruction, ZYDIS_REGISTER_RFLAGS, 64);
	}

	template <typename T>
	void emitPushRegister(Instruction* instruction, T reg, BYTE size)
	{
		switch (reg)
		{
		case ZYDIS_REGISTER_RSP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHRSP64))); return;
		case ZYDIS_REGISTER_ESP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHRSP32))); return;
		case ZYDIS_REGISTER_SP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHRSP16))); return;
		case ZYDIS_REGISTER_SPL: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHRSP8))); return;
		}

		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHR64), getVirtualRegisterIndex(reg), 8)); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHR32), getVirtualRegisterIndex(reg), 8)); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHR16), getVirtualRegisterIndex(reg), 8)); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHR8), getVirtualRegisterIndex(reg), 8)); return;
		}
	}

	template <typename T>
	void emitPopRegister(Instruction* instruction, T reg, BYTE size)
	{
		switch (reg)
		{
		case ZYDIS_REGISTER_RSP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPRSP64))); return;
		case ZYDIS_REGISTER_ESP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPRSP32))); return;
		case ZYDIS_REGISTER_SP: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPRSP16))); return;
		case ZYDIS_REGISTER_SPL: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPRSP8))); return;
		}

		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPR64), getVirtualRegisterIndex(reg), 8)); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPR32), getVirtualRegisterIndex(reg), 8)); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPR16), getVirtualRegisterIndex(reg), 8)); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(POPR8), getVirtualRegisterIndex(reg), 8)); return;
		}
	}

	void emitPushImmediate(Instruction* instruction, long long immediate, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHI64), immediate, 64)); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHI32), immediate, 32)); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHI16), immediate, 16)); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(PUSHI8), immediate, 8)); return;
		}
	}

	void emitRead(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(READ64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(READ32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(READ16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(READ8))); return;
		}
	}

	void emitWrite(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(WRITE64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(WRITE32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(WRITE16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(WRITE8))); return;
		}
	}

	void emitJmp(Instruction* instruction) { instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(JMP), 0x0, 32)); }

	void emitJne(Instruction* instruction) { instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(JNE), 0x0, 32)); }

	void emitExit(Instruction* instruction) { instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(EXIT))); }

	void emitAdd(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(ADD64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(ADD32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(ADD16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(ADD8))); return;
		}
	}

	void emitSub(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SUB64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SUB32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SUB16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SUB8))); return;
		}
	}

	void emitXor(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(XOR64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(XOR32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(XOR16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(XOR8))); return;
		}
	}

	void emitAnd(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(AND64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(AND32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(AND16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(AND8))); return;
		}
	}

	void emitOr(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(OR64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(OR32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(OR16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(OR8))); return;
		}
	}

	void emitShl(Instruction* instruction, BYTE size)
	{
		switch (size)
		{
		case 64: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SHL64))); return;
		case 32: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SHL32))); return;
		case 16: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SHL16))); return;
		case 8: instruction->addVirtualInstruction(VirtualInstruction(getVmHandlerRva(SHL8))); return;
		}
	}

	void emitArithmetic(Instruction* instruction, ZydisMnemonic operation, BYTE size)
	{
		switch (operation)
		{
		case ZYDIS_MNEMONIC_ADD: emitAdd(instruction, size); return;
		case ZYDIS_MNEMONIC_SUB: emitSub(instruction, size); return;
		case ZYDIS_MNEMONIC_XOR: emitXor(instruction, size); return;
		case ZYDIS_MNEMONIC_AND: emitAnd(instruction, size); return;
		case ZYDIS_MNEMONIC_OR: emitOr(instruction, size); return;
		case ZYDIS_MNEMONIC_SHL: emitShl(instruction, size); return;
		}
	}

	void zeroRegister(Instruction* instruction, ZydisRegister reg)
	{
		emitPushImmediate(instruction, 0x0, 64);
		emitPopRegister(instruction, reg, 64);
	}

	void calculateEffectiveAddress(Instruction* instruction, ZydisDecodedOperandMem mem)
	{
		ZydisRegister base = mem.base;
		ZydisRegister index = mem.index;
		BYTE scale = mem.scale;
		bool hasDisplacement = mem.disp.has_displacement;
		long long displacement = mem.disp.value;

		bool is64Bits = (base != ZYDIS_REGISTER_NONE && ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, base) == 64) ||
			(index != ZYDIS_REGISTER_NONE && ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, index) == 64);

		if (base == ZYDIS_REGISTER_RIP)
		{
			emitPushImmediate(instruction, instruction->getRva() + instruction->getInstructionInfo().length + displacement, 64);
			emitPushRegister(instruction, R1, 64);
			emitArithmetic(instruction, ZYDIS_MNEMONIC_ADD, 64);
			emitPopRegister(instruction, R0, 64);
			return;
		}

		if (base != ZYDIS_REGISTER_NONE)
		{
			if (base == ZYDIS_REGISTER_RSP)
			{
				emitPushRegister(instruction, ZYDIS_REGISTER_RSP, 64);
			}
			else
			{
				if (is64Bits)
				{
					emitPushRegister(instruction, base, 64);
				}
				else
				{
					emitPushRegister(instruction, base, 32);
				}
			}
		}
		else
		{
			if (is64Bits)
			{
				emitPushImmediate(instruction, 0x0, 64);
			}
			else
			{
				emitPushImmediate(instruction, 0x0, 32);
			}
		}

		if (index != ZYDIS_REGISTER_NONE)
		{
			if (is64Bits)
			{
				emitPushRegister(instruction, index, 64);
			}
			else
			{
				emitPushRegister(instruction, index, 32);
			}

			if (scale > 1)
			{
				emitPushImmediate(instruction, std::log2(scale), 64);
				if (is64Bits)
				{
					emitArithmetic(instruction, ZYDIS_MNEMONIC_SHL, 64);
				}
				else
				{
					emitArithmetic(instruction, ZYDIS_MNEMONIC_SHL, 32);
				}
				emitPopRegister(instruction, R0, 64);
			}
		}
		else
		{
			if (is64Bits)
			{
				emitPushImmediate(instruction, 0x0, 64);
			}
			else
			{
				emitPushImmediate(instruction, 0x0, 32);
			}
		}

		if (hasDisplacement)
		{
			if (is64Bits)
			{
				emitPushImmediate(instruction, displacement, 64);
			}
			else
			{
				emitPushImmediate(instruction, displacement, 32);
			}
		}
		else
		{
			if (is64Bits)
			{
				emitPushImmediate(instruction, 0x0, 64);
			}
			else
			{
				emitPushImmediate(instruction, 0x0, 32);
			}
		}

		if (is64Bits)
		{
			emitArithmetic(instruction, ZYDIS_MNEMONIC_ADD, 64);
			emitPopRegister(instruction, R0, 64);
			emitArithmetic(instruction, ZYDIS_MNEMONIC_ADD, 64);
			emitPopRegister(instruction, R0, 64);
		}
		else
		{
			emitArithmetic(instruction, ZYDIS_MNEMONIC_ADD, 32);
			emitPopRegister(instruction, R0, 32);
			emitArithmetic(instruction, ZYDIS_MNEMONIC_ADD, 32);
			emitPopRegister(instruction, R0, 32);
		}
	}
}