#pragma once

#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS
//#define CV_LOG_STRIP_LEVEL CV_LOG_LEVEL_VERBOSE + 1

#include <iostream>
#include <string>
#include <fstream>
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
	int HASH_THREAD_COUNT = 1024;
	const int MAX_KEY_SIZE = 17; //16
	const int HASH_UINT_COUNT = 8;
	const int HASH_UINT_SIZE = HASH_UINT_COUNT * sizeof(cl_uint);
	const int HASH_CHAR_SIZE = 65 * sizeof(char);

	cl::vector<Platform> platforms;
	cl::vector<Device> devices;
	CommandQueue queue;
	Context context;
	Program program;
	Kernel kernel;
	int deviceId;
    
public:
	GPUController();
	GPUController(int);

	void platform();

	void singleHash(std::string key);
	void singleHashSalted(std::string key, std::string salt);
	void multiHash(std::string sourceFile, std::string targetFile);

	void crackSingle(std::string sourceFile, std::string hash);
	void crackSingleSalted(std::string sourceFile, std::string hash);
	void crackSingleSaltedBulk(std::string sourceFile, std::string hash, unsigned int count);

private:
	bool compileKernel(std::string, std::string, std::string = "");
	cl_uint* hexdec(const char* hex);
};