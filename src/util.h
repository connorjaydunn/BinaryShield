#pragma once

#include <windows.h>
#include <vector>
#include <random>

template <typename T>
std::vector<BYTE> convertToByteVector(T x)
{
    std::vector<BYTE> bytes;
    size_t byteCount = sizeof(T);

    for (size_t i = 0; i < byteCount; ++i)
        bytes.push_back((BYTE)((x >> (i * 8)) & 0xFF));

    return bytes;
}

int getRandomInt(int min, int max);

DWORD align(DWORD x, DWORD alignment);

DWORD rvaToFileOffset(DWORD rva, DWORD virtualAddress, DWORD pointerToRawData);

DWORD fileOffsetToRva(DWORD fileOffset, DWORD virtualAddress, DWORD pointerToRawData);