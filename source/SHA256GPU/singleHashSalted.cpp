#include "GPUController.hpp"

void GPUController::singleHashSalted(std::string key, std::string salt)
{
	try
	{
		//Compile kernel
		if (!compileKernel("./kernels/hash_single_salt.kernel.cl", "sha256salted_kernel"))
		{
			return;
		}

		//Prepare input
		cl_uint saltLength = salt.length();
		const char* csalt = salt.c_str();
		cl_uint keyLength = key.length();
		const char* ckey = key.c_str();

		//Create memory buffers
		Buffer saltBuffer = Buffer(context, CL_MEM_READ_ONLY, saltLength);
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, keyLength + saltLength);
		Buffer hashOutBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_CHAR_SIZE);

		//Write data on input buffers!
		queue.enqueueWriteBuffer(saltBuffer, CL_TRUE, 0, saltLength, csalt);
		queue.enqueueWriteBuffer(keyBuffer, CL_TRUE, 0, keyLength, ckey);

		//Set arguments to kernel
		kernel.setArg(0, saltLength);
		kernel.setArg(1, saltBuffer);
		kernel.setArg(2, keyLength);
		kernel.setArg(3, keyBuffer);
		kernel.setArg(4, hashOutBuffer);

		//Run kernel
		NDRange _global_(1);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);

		//Read result
		char result[65];
		queue.enqueueReadBuffer(hashOutBuffer, CL_TRUE, 0, HASH_CHAR_SIZE, result);

		//Print result
		printf("%s\n", result);
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}