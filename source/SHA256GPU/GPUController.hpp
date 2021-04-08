#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS
//#define CV_LOG_STRIP_LEVEL CV_LOG_LEVEL_VERBOSE + 1

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

//Namespaces
using namespace cl;
using namespace std::chrono;



class GPUController
{
private:
	//Constants
	const size_t MAX_KEY_SIZE = 24 + 1; //24: char, 1: (c string null)
	const size_t HASH_UINT_COUNT = 8; //256 bit / 32 bit ints
	const size_t HASH_UINT_SIZE = HASH_UINT_COUNT * sizeof(cl_uint); //byte size of hash uints array
	const size_t HASH_CHAR_SIZE = 65 * sizeof(char); //64: (hash) + 1: (c string null)
	
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

	bool attachDevice(const int contextId = 0, const int deviceId = 0, const int threadSize = 1000);

	std::string platformDetails() const;

	//Hashing
	std::string hashSingle(std::string key);
	std::string hashSingleSalted(std::string key, std::string salt);
	unsigned int hashMultiple(std::string sourceFile, std::string targetFile);

	//Cracking
	std::string crackSingle(const std::string sourceFile, const std::string hash);
	std::string crackSingleSalted(std::string sourceFile, std::string hash);

private:
	std::string compileKernel(const std::string fileName, const std::string kernelName, const std::string parameters = "");
	void hexToDec(const std::string hex, cl_uint* dec) const;
};