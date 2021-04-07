#include "GPUController.hpp"

std::string GPUController::hashSingleSalted(std::string key, std::string salt)
{
	std::string concat = key + salt;
	printf("Key: '%s' + '%s' -> '%s'\n", key.c_str(), salt.c_str(), concat.c_str());

	return hashSingle(concat);
}