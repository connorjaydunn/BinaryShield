#pragma once

#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

#include "vm_section.h"
#include "function.h"
#include "util.h"

class PE
{
public:
	PE(std::string path);
	~PE();

	bool load();
	bool save(std::string path);

	void addFunctionByRva(DWORD startRva, DWORD endRva);
	bool virtualizeFunctions();
	bool addVmSection();

	DWORD rvaToFileOffset(DWORD rva);
	DWORD fileOffsetToRva(DWORD offset);
private:
	std::string path;
	HANDLE hFile = nullptr;
	std::vector<BYTE> bytes;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_SECTION_HEADER pSectionHeader;
	bool parseHeaders();
	bool open();
	void close();
	bool emitRead();
	bool addSection(std::string name, DWORD flags, std::vector<BYTE> bytes);
	DWORD getNewSectionVirtualAddress();
	DWORD getNewSectionFileOffset();

	std::vector<Function> functions;
	bool virtualizeFunction(Function function);
	void redirectFunctionToVmTramp(Function function, DWORD vmTrampRva);
	void removeOriginalFunctionBytes(Function function);

	VMSection vmSection;
};