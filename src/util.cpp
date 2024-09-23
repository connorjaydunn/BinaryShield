#include "util.h"

DWORD align(DWORD x, DWORD alignment) { return (x + alignment - 1) & ~(alignment - 1); }

DWORD rvaToFileOffset(DWORD rva, DWORD virtualAddress, DWORD pointerToRawData) { return (rva - virtualAddress) + pointerToRawData; }

DWORD fileOffsetToRva(DWORD fileOffset, DWORD virtualAddress, DWORD pointerToRawData) { return (fileOffset - pointerToRawData) + virtualAddress; }