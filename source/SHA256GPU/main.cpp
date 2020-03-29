#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS

#define CV_LOG_STRIP_LEVEL CV_LOG_LEVEL_VERBOSE + 1

#include <iostream>
#include <string>
#include <fstream>

#include <CL/cl.hpp>
#include <utility>
#include <oclutils.hpp>

using cl::vector;
using namespace cl;

void printPlatformDetails();
void createHash(string);
void createHashes(string);

const int KEY_SIZE = 64;
const int HASH_SIZE = 64;
const int HASH_RESULT_SIZE = 8;

int main(int argc, const char* argv[])
{
	/*if (argc == 1)
	{
		printf("platform                               : list platforms\n");
		printf("hash single <password>                 : hash a single password\n");
		printf("hash multiple <input.txt> <output.txt> : hash a single password\n");
		return 0;
	}*/

	

	string key = "banana";
	createHash(key);

	//string file = "../passwords/passwords-100.txt";
	//createHashes(file);

	return 0;
}

void printPlatformDetails()
{
	try
	{
		// Get available platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

		std::cout << "Platform count: " << platforms.size() << std::endl;

		vector<Device> devices;
		Context context;

		for (Platform p : platforms)
		{
			try
			{
				std::cout << p.getInfo<CL_PLATFORM_NAME>() << std::endl;
				std::cout << p.getInfo<CL_PLATFORM_VERSION>() << std::endl;

				// Select the default platform and create a context using this platform and the GPU
				// These are key-value pairs.
				cl_context_properties cps[3] =
				{
					CL_CONTEXT_PLATFORM,
					(cl_context_properties)(p)(),
					0
				};

				context = Context(CL_DEVICE_TYPE_GPU, cps);

				// Get a list of devices on this platform
				devices = context.getInfo<CL_CONTEXT_DEVICES>();

				std::cout << "Number of devices: " << devices.size() << std::endl << std::endl;

				for (const auto& device : devices)
				{
					std::cout << "### Device" << std::endl;
					std::cout << "\tName: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
					std::cout << "\tMemory size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024.0f / 1024.0f << "MB" << std::endl;
					std::cout << "\tCache type: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>() << std::endl;
					std::cout << "\tCache size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() / 1024.0f << "KB" << std::endl;
					std::cout << "\tCacheline size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>() << std::endl;
				}
			}
			catch (Error error)
			{
				oclPrintError(error);
				continue;
			}

		}

		if (devices.size() == 0)
		{
			throw Error(CL_INVALID_CONTEXT, "Failed to create a valid context!");
		}

	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}

void createHash(string key)
{
	try
	{
		// Get available platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

		vector<Device> devices;
		Context context;

		for (Platform p : platforms)
		{

			try
			{
				// Select the default platform and create a context using this platform and the GPU
				cl_context_properties cps[3] =
				{
					CL_CONTEXT_PLATFORM,
					(cl_context_properties)(p)(),
					0
				};

				context = Context(CL_DEVICE_TYPE_GPU, cps);
				devices = context.getInfo<CL_CONTEXT_DEVICES>();
			}
			catch (Error error)
			{
				oclPrintError(error);
				continue;
			}

			if (devices.size() > 0)
				break;
		}

		if (devices.size() == 0)
		{
			throw Error(CL_INVALID_CONTEXT, "Failed to create a valid context!");
		}

		//Command queue on the first device
		CommandQueue queue = CommandQueue(context, devices[0]);

		//Compile kernel
		std::ifstream sourceFile("hash_single.kernel.cl");
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			                  (std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		Program program = Program(context, source);
		program.build(devices); //Configure for current device

		//Kernel target
		Kernel kernel(program, "sha256kernel");

		//Prepare input
		cl_uint length = key.length();
		const char* ckey = key.c_str();

		// Create memory buffers
		Buffer buffer1 = Buffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint));
		Buffer buffer2 = Buffer(context, CL_MEM_READ_ONLY, length);
		Buffer buffer3 = Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * HASH_RESULT_SIZE);

		// Write data on input buffers!
		queue.enqueueWriteBuffer(buffer1, CL_TRUE, 0, sizeof(cl_uint), &length);
		queue.enqueueWriteBuffer(buffer2, CL_TRUE, 0, length, ckey);

		// Set arguments to kernel
		kernel.setArg(0, buffer1);
		kernel.setArg(1, buffer2);
		kernel.setArg(2, buffer3);

		// Run the kernel on specific ND range
		NDRange _global_(1);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);

		// Read buffer C (the result) into a local piece of memory
		cl_uint* result = new cl_uint[sizeof(cl_uint) * HASH_RESULT_SIZE];
		queue.enqueueReadBuffer(buffer3, CL_TRUE, 0, sizeof(cl_uint) * HASH_RESULT_SIZE, result);

		//Assemble result
		std::cout << result;
		char* out = new char[65];
		for (int i = 0; i < HASH_RESULT_SIZE; i++)
		{
			sprintf(out + i*8, "%08x", result[i]);
		}
		printf("\nHash: %s", out);
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}

void createHashes(string file)
{
	try
	{
		// Get available platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

		vector<Device> devices;
		Context context;

		for (auto p : platforms)
		{

			try
			{
				// Select the default platform and create a context using this platform and the GPU
				cl_context_properties cps[3] =
				{
					CL_CONTEXT_PLATFORM,
					(cl_context_properties)(p)(),
					0
				};

				context = Context(CL_DEVICE_TYPE_GPU, cps);
				devices = context.getInfo<CL_CONTEXT_DEVICES>();
			}
			catch (Error error)
			{
				oclPrintError(error);
				continue;
			}

			if (devices.size() > 0)
				break;
		}

		if (devices.size() == 0)
		{
			throw Error(CL_INVALID_CONTEXT, "Failed to create a valid context!");
		}

		//Command queue on the first device
		CommandQueue queue = CommandQueue(context, devices[0]);

		//Compile kernel
		std::ifstream sourceFile("hash_kernel.cl");
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		Program program = Program(context, source);
		program.build(devices); //Configure for current device

		//Kernel target
		Kernel kernel(program, "sha256kernel");

		// Create memory buffers
		// 2 × input buffer, 1 × output buffer
		const char target[] = "banana";
		int s = sizeof(target);
		Buffer buffer1 = Buffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * 3);
		Buffer buffer2 = Buffer(context, CL_MEM_READ_ONLY, sizeof(target));
		Buffer buffer3 = Buffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * HASH_RESULT_SIZE);

		// Copy lists A and B to the memory buffers
		/// Write data on input buffers!
		unsigned int args[] =
		{
			64, 1, sizeof(target) - 1
		};

		queue.enqueueWriteBuffer(buffer1, CL_TRUE, 0, sizeof(unsigned int) * 3, args);
		queue.enqueueWriteBuffer(buffer2, CL_TRUE, 0, sizeof(target), target);

		// Set arguments to kernel
		kernel.setArg(0, buffer1);
		kernel.setArg(1, buffer2);
		kernel.setArg(2, buffer3);

		// Run the kernel on specific ND range
		NDRange _global_(1);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);

		// Read buffer C (the result) into a local piece of memory
		cl_uint* C = new cl_uint[sizeof(cl_uint) * HASH_RESULT_SIZE];
		queue.enqueueReadBuffer(buffer3, CL_TRUE, 0, sizeof(cl_uint) * HASH_RESULT_SIZE, C);

		//Assemble result
		std::cout << C;
		char* out = new char[65];
		for (int i = 0; i < HASH_RESULT_SIZE; i++)
		{
			sprintf(out + i * 8, "%08x", C[i]);
		}
		printf("\nHash: %s", out);
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}