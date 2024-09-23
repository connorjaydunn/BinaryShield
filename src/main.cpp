#include <iostream>
#include <cstdlib>
#include <string>

#include "pe.h"

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cerr << "usage: binaryshield.exe <target binary path> <start-rva> <end-rva>" << std::endl;
		return 1;
	}

	std::cout << "starting..." << std::endl;

	PE pe(argv[1]);

	if (!pe.load())
		return 1;

	pe.addFunctionByRva(std::stoi(argv[2], nullptr, 16), std::stoi(argv[3], nullptr, 16));

	std::cout << "virtualizing function(s)..." << std::endl;

	if (!pe.virtualizeFunctions())
		return 1;

	std::cout << "success" << std::endl;

	if (!pe.addVmSection())
		return 1;

	if (!pe.save("protected.exe"))
		return 1;

	std::cout << "exiting..." << std::endl;

	return 0;
}