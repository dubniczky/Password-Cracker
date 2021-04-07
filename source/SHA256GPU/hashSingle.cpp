#include "GPUController.hpp"

std::string GPUController::hashSingle(std::string key)
{
	
	try
	{
		//Compile kernel
		printf("Compiling kernel...\n");
		std::string res; 
		if ((res = compileKernel("hash_single.kernel.cl", "sha256single_kernel")) != "")
		{
			return res;
		}
		printf("Kernel compiled.\n");

		
		//Prepare input
		cl_uint length = key.length();
		const char* ckey = key.c_str();

		//Create memory buffers
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, length);
		Buffer outBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_CHAR_SIZE);
		

		//Write data on input buffers
		queue.enqueueWriteBuffer(keyBuffer, CL_TRUE, 0, length, ckey);

		//Set arguments to kernel
		kernel.setArg(0, length);
		kernel.setArg(1, keyBuffer);
		kernel.setArg(2, outBuffer);

		//Run kernel
		NDRange _global_(1);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange, nullptr);

		//Read result
		char result[65];
		queue.enqueueReadBuffer(outBuffer, CL_TRUE, 0, HASH_CHAR_SIZE, result);

		//Print result
		printf("%s\n", result);
		return std::string(result);
	}
	catch (Error error)
	{
		oclPrintError(error);
		return std::string("errors");
	}
}