#pragma once

#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS
//#define CV_LOG_STRIP_LEVEL CV_LOG_LEVEL_VERBOSE + 1

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdio>

#include <CL/cl.hpp>
#include <oclutils.hpp>

using cl::vector;
using namespace cl;
using namespace std::chrono;

class GPUController
{
private:
	//Constants
	const size_t MAX_KEY_SIZE = 16 + 1; //16 char, 1 null
	const size_t HASH_UINT_COUNT = 8;
	const size_t HASH_UINT_SIZE = HASH_UINT_COUNT * sizeof(cl_uint);
	const size_t HASH_CHAR_SIZE = (64 * sizeof(char)) + (1 * sizeof(char)); //64: hash, 1: null
	
	//Assigned by contructor
	cl::Platform platform;
	cl::Device device;	
	cl::Context context;
	cl::CommandQueue queue;
	int platformId;
	int deviceId;

	//Assigned by kernel compiler
	cl::Program program;
	cl::Kernel kernel;
	int threadSize;
	
    
public:
	//Constructor
	GPUController();
	GPUController(int contextId, int deviceId, int threadSize = 1000);


	void platformDetails() const;
	bool attachDevice(const int contextId = 0, const int deviceId = 0, const int threadSize = 1000);

	//Hashing
	std::string singleHash(std::string key);
	std::string singleHashSalted(std::string key, std::string salt);
	unsigned int multiHash(std::string sourceFile, std::string targetFile);

	//Cracking
	std::string crackSingle(const std::string sourceFile, const std::string hash);
	std::string crackSingleSalted(std::string sourceFile, std::string hash);

private:
	bool compileKernel(const std::string fileName, const std::string kernelName, const std::string parameters = "");
	void hexToDec(const std::string hex, cl_uint* dec) const;
};