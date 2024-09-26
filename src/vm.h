#pragma once

#include <windows.h>
#include <vector>
#include <Zydis/Zydis.h>

#include "vm_handler.h"
#include "virtual_instruction.h"
#include "util.h"

class Instruction;

namespace VM
{
	enum VIRTUAL_REGISTERS
	{
		R0, // general purpose register
		R1, // module base
	};

	extern std::vector<VMHandler> vmHandlers;

	bool compileInstructionToVirtualInstructions(Instruction* instruction);

	bool x86PushHandler(Instruction* instruction);
	bool x86PopHandler(Instruction* instruction);
	bool x86MovHandler(Instruction* instruction);
	bool x86LeaHandler(Instruction* instruction);
	bool x86RetHandler(Instruction* instruction);
	bool x86CmpHandler(Instruction* instruction);
	bool x86TestHandler(Instruction* instruction);
	bool x86JmpHandler(Instruction* instruction);
	bool x86JneHandler(Instruction* instruction);
	bool x86ArithmeticHandler(Instruction* instruction, ZydisMnemonic operation, bool storeOutput);

	BYTE getVirtualRegisterIndex(ZydisRegister reg);
	BYTE getVirtualRegisterIndex(VIRTUAL_REGISTERS reg);
	DWORD getVmHandlerRva(VMHandlerTypes type);

	void popVmContext(Instruction* instruction);
	void pushVmContext(Instruction* instruction);

	template <typename T>
	void emitPushRegister(Instruction* instruction, T reg, BYTE size);
	template <typename T>
	void emitPopRegister(Instruction* instruction, T reg, BYTE size);
	void emitPushImmediate(Instruction* instruction, long long immediate, BYTE size);
	void emitRead(Instruction* instruction, BYTE size);
	void emitWrite(Instruction* instruction, BYTE size);
	void emitJmp(Instruction* instruction);
	void emitJne(Instruction* instruction);
	void emitExit(Instruction* instruction);
	void emitAdd(Instruction* instruction, BYTE size);
	void emitSub(Instruction* instruction, BYTE size);
	void emitXor(Instruction* instruction, BYTE size);
	void emitAnd(Instruction* instruction, BYTE size);
	void emitOr(Instruction* instruction, BYTE size);
	void emitShl(Instruction* instruction, BYTE size);
	void emitArithmetic(Instruction* instruction, ZydisMnemonic operation, BYTE size);
	void zeroRegister(Instruction* instruction, ZydisRegister reg);
	void calculateEffectiveAddress(Instruction* instruction, ZydisDecodedOperandMem mem);
}