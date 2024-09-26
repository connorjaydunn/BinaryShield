#include "util.h"

int getRandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> distribution(min, max);

	return distribution(generator);
}

DWORD align(DWORD x, DWORD alignment) { return (x + alignment - 1) & ~(alignment - 1); }

DWORD rvaToFileOffset(DWORD rva, DWORD virtualAddress, DWORD pointerToRawData) { return (rva - virtualAddress) + pointerToRawData; }

DWORD fileOffsetToRva(DWORD fileOffset, DWORD virtualAddress, DWORD pointerToRawData) { return (fileOffset - pointerToRawData) + virtualAddress; }