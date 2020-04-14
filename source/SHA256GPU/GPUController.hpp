#pragma once

#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS
//#define CV_LOG_STRIP_LEVEL CV_LOG_LEVEL_VERBOSE + 1

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

#include <CL/cl.hpp>
//#include <utility>
#include <oclutils.hpp>

using cl::vector;
using namespace cl;
using namespace std::chrono;

class GPUController
{
private:
	static const int HASH_THREAD_COUNT = 256;
	static const int HASH_RESULT_SIZE = 8;
public:
	static void platform();
	static void singleHash(string);
	static void singleHashSalted(string, string);
	static void multiHash(string, string);
};