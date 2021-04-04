#include "GPUController.hpp"

std::string GPUController::singleHashSalted(std::string key, std::string salt)
{
	std::string concat = key + salt;
	printf("Key: '%s' + '%s' -> '%s'\n", key.c_str(), salt.c_str(), concat.c_str());

	return singleHash(concat);
}