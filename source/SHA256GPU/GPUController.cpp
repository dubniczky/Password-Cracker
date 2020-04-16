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
bool GPUController::compileKernel(string fileName, string kernelName)
{
	try
	{
		std::ifstream sourceFile(fileName.c_str());
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = Program(context, source);
		program.build(devices);
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

void GPUController::platform()
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
					std::cout << "\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz" << std::endl;
					std::cout << "\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
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

void GPUController::singleHash(string key)
{
	try
	{
		//Compile kernel
		if (!compileKernel("hash_single.kernel.cl", "sha256single_kernel"))
		{
			return;
		}

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
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}
void GPUController::singleHashSalted(string key, string salt)
{
	try
	{
		//Compile kernel
		if (!compileKernel("hash_single_salt.kernel.cl", "sha256salted_kernel"))
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
		Buffer hashOutBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_UINT_SIZE);

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
		queue.enqueueReadBuffer(hashOutBuffer, CL_TRUE, 0, HASH_UINT_SIZE, result);

		//Print result
		printf("%s\n", result);
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}

void GPUController::multiHash(string infileName, string outfileName)
{
	int hashThreadCount = HASH_THREAD_COUNT;
	Program program;
	vector<Device> devices;
	try
	{
		printf("Compiling kernel...\n");
		// Get available platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

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
		std::ifstream sourceFile("hash_multiple.kernel.cl");
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = Program(context, source);
		program.build(devices); //Configure for current device

		//Kernel target
		Kernel kernel(program, "sha256kernel_multiple");
		printf("Kernel compiled.\n");

		//Prepare input
		printf("Reading file...\n");
		std::vector<std::string> lines;
		std::string line;
		std::ifstream infile(infileName.c_str());
		while (std::getline(infile, line))
		{
			lines.push_back(line);
		}
		infile.close();
		printf("Read %d lines.\n", lines.size());

		int iterations = (lines.size() / hashThreadCount) + (lines.size() / hashThreadCount > 0 ? 1 : 0);
		//cl::Event* events = new cl::Event[iterations];

		printf("Starting kernel...\n");

		std::ofstream outFile(outfileName.c_str());
		auto startTime = high_resolution_clock::now();
		for (int i = 0; i < iterations; i++)
		{
			int start = i * hashThreadCount;
			if (i == iterations - 1)
			{
				hashThreadCount = lines.size() % hashThreadCount;
			}

			//Longest string
			cl_uint longest = 0;
			for (int j = 0; j < hashThreadCount; j++)
			{
				if (lines[start + j].length() > longest)
				{
					longest = lines[start + j].length();
				}
			}
			longest += 1;
			cl_uint bufferSize = hashThreadCount * longest;

			//Prepare buffer
			char* strBuffer = new char[bufferSize];
			for (int j = 0; j < hashThreadCount; j++)
			{
				strcpy(&strBuffer[j * longest], lines[start + j].c_str());
			}

			// Create memory buffers
			Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, bufferSize);
			Buffer hashOutBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_UINT_SIZE * hashThreadCount);

			// Write data on input buffers!
			queue.enqueueWriteBuffer(keyBuffer, CL_TRUE, 0, bufferSize, strBuffer);

			// Set arguments to kernel
			kernel.setArg(0, longest);
			kernel.setArg(1, keyBuffer);
			kernel.setArg(2, hashOutBuffer);

			//Run kernel
			NDRange _global_(HASH_THREAD_COUNT);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);
			

			cl_uint* result = new cl_uint[HASH_UINT_SIZE * hashThreadCount];
			queue.enqueueReadBuffer(hashOutBuffer, CL_TRUE, 0, HASH_UINT_SIZE* hashThreadCount, result);

			for (int i = 0; i < hashThreadCount; i++)
			{
				char* out = new char[65];
				for (int j = 0; j < HASH_UINT_SIZE; j++)
				{
					sprintf(out + j * 8, "%08x", result[(i * 8) + j]);
				}
				outFile << out << std::endl;
			}
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stopTime - startTime);

		printf("Kernel completed.\n");
		printf("Runtime: %d microseconds.\n", (int)duration.count());

		printf("Saving result to file...\n");
		outFile.close();
		printf("File saved.\n");

		//delete[] events;
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
	}
}