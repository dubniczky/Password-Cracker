#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS

//Base
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdio>

//OpenCL
#include <CL/cl.hpp>
#include <oclutils.hpp>

//Projects
#include "KernelProperties.hpp"

//Namespaces
using namespace cl;
using namespace std::chrono;


class GPUController
{
private:
	//Constants
	unsigned int MAX_KEY_SIZE = 24 + 1; //24: char, 1: (c string null)
	const size_t HASH_UINT_COUNT = 8; //256 bit / 32 bit ints
	const size_t HASH_UINT_SIZE = HASH_UINT_COUNT * sizeof(cl_uint); //byte size of hash uints array
	const size_t HASH_CHAR_SIZE = 65 * sizeof(char); //64: (hash) + 1: (c string null)
	
	
	cl::Platform platform; //Device platform
	cl::Device device; //Used device
	cl::Context context; //Device context
	cl::CommandQueue queue; //Current command queue
	int platformId = 0; //Platform identifier
	int deviceId = 0; //Device identifier
	cl::Program program; //Compiled kernel program
	cl::Kernel kernel; //Compiled kernel
	int threadSize = 0; //Number of keys in one thread
	
    
public:
	/**
	 * Creates an instance of GPUController. Must call attachDevice before using!
	 */
	GPUController();

	/**
	  * Creates an instance of GPUController and attaches the specified device.
	  * @param props The propterties to attach the device with.
	 */
	GPUController(const KernelProperties& props);

	/**
	 * Deletes an instance of GPUController.
	 */
	~GPUController()
	{
		
	}


	/**
	  * Attaches the specified device to the instance.
	  * @param props The propterties to attach the device with.
	  * @return Device successfully attached.
	 */
	bool attachDevice(const KernelProperties& props);



	/**
	  * Prints the details of the available devices in the system
	 */
	std::string platformDetails() const;



	//Hashing
	/**
	  * Hashes a single key on the GPU. Do not call repeatedly, use hashMultiple instead.
	  * @param key Key to hash.
	  * @return The hash in hex string form.
	 */
	std::string hashSingle(const std::string& key);

	/**
	  * Hashes a single key with salt on the GPU.
	  * @param key Key to hash.
	  * @param salt Salt to to hash the key with.
	  * @return The hash in hex string form (salt is not attached).
	 */
	std::string hashSingleSalted(const std::string& key, const std::string& salt);

	/**
	  * Reads keys from a file and writes the hashes into a target output.
	  * @param sourceFile Path of the key file.
	  * @param targetFile Path of the target file.
	  * @return Number of keys hashed.
	 */
	unsigned int hashMultiple(const std::string& sourceFile, const std::string& targetFile);



	//Cracking
	/**
	  * Reads keys from a file and tries to crack the given hash using them.
	  * @param sourceFile File with the key table to try.
	  * @param hash Hash to crack.
	  * @return The key if successful otherwise empty string.
	 */
	std::string crackSingle(const std::string& sourceFile, const std::string& hash);

	/**
	  * Reads keys from a file and tries to crack the given hash with salt using them.
	  * @param sourceFile File with the key table to try.
	  * @param hash Hash to crack (salt appended to the beginning).
	  * @return The key if successful otherwise empty string.
	 */
	std::string crackSingleSalted(const std::string& sourceFile, const std::string& hash);

private:
	/**
	  * Compiles the specified kernel and attaches it to the object.
	  * @param fileName File path of the main kernel.
	  * @param kernelName Name of the main kernel method.
	  * @param parameters Additional OpenCL compiler parameters.
	  * @return Empty string if successful, otherwise the error message.
	 */
	std::string compileKernel(const std::string fileName, const std::string kernelName, const std::string parameters = "");

	/**
	  * Convert a hex string to an unsigned integer array.
	  * @param hex Hex string.
	  * @param dec Name of the main kernel method.
	  * @return Whether the input was a valid hex string.
	 */
	bool hexToDec(const std::string hex, cl_uint* dec) const;
};