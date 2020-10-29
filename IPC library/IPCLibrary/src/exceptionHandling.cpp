#include "..\IPCLibrary.h"

void exception::error(std::string modual, std::string msg) {
	std::cerr << "[ERROR] (" << modual << ") " << msg << std::endl;
}

void exception::warning(std::string modual, std::string msg)
{
	std::cerr << "[WARNING] (" << modual << ") " << msg << std::endl;
}

