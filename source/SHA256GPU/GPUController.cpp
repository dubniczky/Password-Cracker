#include "GPUController.hpp"

GPUController::GPUController() : GPUController(0)
{
	
}
GPUController::GPUController(int deviceId)
{
	this->deviceId = deviceId;

	Platform::get(&platforms);

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

	queue = CommandQueue(context, devices[deviceId], CL_QUEUE_PROFILING_ENABLE);
}

bool GPUController::compileKernel(string fileName, string kernelName, std::string params)
{
	try
	{
		std::ifstream sourceFile(fileName.c_str());
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = Program(context, source);
		program.build(devices, params.c_str());
		kernel = Kernel(program, kernelName.c_str());
		return true;
	}
	catch (Error error)
	{
		oclPrintError(error);
		if (error.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			for (cl::Device dev : devices)
			{
				// Check the build status
				cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
				std::cerr << "Build log for " << name << ":" << std::endl
					<< buildlog << std::endl;
			}
		}
		return false;
	}
}
cl_uint* GPUController::hexdec(const char* hex)
{
	cl_uint* res = new cl_uint[HASH_UINT_COUNT];
	char chunk[8];

	#pragma unroll
	for (int i = 0; i < HASH_UINT_COUNT; i++)
	{
		memcpy(&chunk, &hex[i*8], sizeof(char) * 8);
		sscanf(chunk, "%x", &res[i]);
		//printf("%u ", &res[i]);
	}
	//printf("\n");
	return res;
}