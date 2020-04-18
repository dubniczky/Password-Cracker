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
	int HASH_THREAD_COUNT = 256;
	const int MAX_KEY_SIZE = 16;
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

	void singleHash(string key);
	void singleHashSalted(string key, string salt);
	void multiHash(string sourceFile, string targetFile);

	void crackSingle(string sourceFile, string hash);

private:
	bool compileKernel(string, string);
	cl_uint* hexdec(const char* hex);
};