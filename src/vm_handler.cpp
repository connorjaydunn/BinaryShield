#include "vm_handler.h"

VMHandlerTypes VMHandler::getType() { return type; }

DWORD VMHandler::getRva() { return rva; }

DWORD VMHandler::getFileOffset() { return fileOffset; }

std::vector<BYTE> VMHandler::getBytes() { return bytes; }

void VMHandler::setRva(DWORD rva) { this->rva = rva; }

void VMHandler::setFileOffset(DWORD fileOffset) { this->fileOffset = fileOffset; }