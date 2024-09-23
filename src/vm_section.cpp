#include "vm_section.h"

void VMSection::initialise(DWORD virtualAddress, DWORD pointerToRawData)
{
	this->virtualAddress = virtualAddress;
	this->pointerToRawData = pointerToRawData;
	addVmHandlers();
	initialised = true;
}

bool VMSection::isInitialised() { return initialised; }

DWORD VMSection::getWritePointer() { return writePointer; }

DWORD VMSection::getWritePointerFileOffset() { return writePointer + pointerToRawData; }

DWORD VMSection::getWritePointerRva() { return fileOffsetToRva(getWritePointerFileOffset(), virtualAddress, pointerToRawData); }

std::vector<BYTE> VMSection::getBytes() { return bytes; }

void VMSection::addVmTramp(DWORD bytecodeRva)
{
	addBytes({ 0x68 }); addBytes(convertToByteVector<DWORD>(bytecodeRva)); // push bytecodeRva

	DWORD relToEnterHandler = VM::getVmHandlerRva(ENTER) - fileOffsetToRva(writePointer + pointerToRawData + 5, virtualAddress, pointerToRawData);

	if (relToEnterHandler <= 127 && relToEnterHandler >= -128)
	{
		addBytes({ 0xEB, (BYTE)(relToEnterHandler & 0xFF) }); // jmp short vmenter
	}
	else
	{
		addBytes({ 0xE9 }); addBytes(convertToByteVector<DWORD>(relToEnterHandler)); // jmp far vmenter
	}
}

void VMSection::addBytes(std::vector<BYTE> bytes)
{
	this->bytes.insert(this->bytes.begin() + writePointer, bytes.begin(), bytes.end());
	writePointer += bytes.size();
}

void VMSection::addVmHandlers()
{
	// there is likely an elegant way of doing this

	VMHandler* vmHandlers[] =
	{
	new Enter(), new Exit(),
	new PushR64(), new PushR32(), new PushR16(), new PushR8(),
	new PopR64(), new PopR32(), new PopR16(), new PopR8(),
	new PushI64(), new PushI32(), new PushI16(), new PushI8(),
	new PushRSP64(), new PushRSP32(), new PushRSP16(), new PushRSP8(),
	new PopRSP64(), new PopRSP32(), new PopRSP16(), new PopRSP8(),
	new Add64(), new Add32(), new Add16(), new Add8(),
	new Sub64(), new Sub32(), new Sub16(), new Sub8(),
	new Xor64(), new Xor32(), new Xor16(), new Xor8(),
	new And64(), new And32(), new And16(), new And8(),
	new Or64(), new Or32(), new Or16(), new Or8(),
	new Nand64(), new Nand32(), new Nand16(), new Nand8(),
	new Nor64(), new Nor32(), new Nor16(), new Nor8(),
	new Shl64(), new Shl32(), new Shl16(), new Shl8(),
	new Read64(), new Read32(), new Read16(), new Read8(),
	new Write64(), new Write32(), new Write16(), new Write8(),
	new Jmp(),
	new Jne(),
	};

	for (int i = 0; i < std::size(vmHandlers); i++)
		addVmHandler(*vmHandlers[i]);

	for (int i = 0; i < std::size(vmHandlers); i++)
		delete vmHandlers[i];
}

void VMSection::addVmHandler(VMHandler vmHandler)
{
	vmHandler.setFileOffset(writePointer + pointerToRawData);
	vmHandler.setRva(fileOffsetToRva(writePointer + pointerToRawData, virtualAddress, pointerToRawData));
	VM::vmHandlers.push_back(vmHandler);
	addBytes(vmHandler.getBytes());
}