#include "..\IPCLibrary.h"

void exception::error(std::string modual, std::string msg) {
	std::cerr << "[ERROR] (" << modual << ") \a\a\a\a\a" << msg << std::endl;
	abort();
}

void exception::warning(std::string modual, std::string msg)
{
	std::cerr << "[WARNING] (" << modual << ") \a" << msg << std::endl;
}

