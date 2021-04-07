#include "GPUController.hpp"

//Construct
GPUController::GPUController() : platformId(0), deviceId(0), threadSize(22400)
{
	
}
GPUController::GPUController(int platformId, int deviceId, int threadSize)
	: platformId(platformId), deviceId(deviceId), threadSize(threadSize)
{
	
}

//Initialize
bool GPUController::attachDevice(const int platformId, const int deviceId, const int threadSize)
{
	//Guard
	if (platformId < 0) throw Error(CL_INVALID_CONTEXT, "Invalid platform ID.");
	if (deviceId < 0) throw Error(CL_INVALID_CONTEXT, "Invalid device ID.");
	if (threadSize < 1) throw Error(CL_INVALID_CONTEXT, "Invalid thread size.");


	//Get device
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;
	Context context;


	//Validate requested device
	try
	{
		//Get platform
		Platform::get(&platforms);
		cl_context_properties cps[3] =
		{
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[platformId])(),
			(cl_context_properties)0
		};

		//Get device
		context = Context(CL_DEVICE_TYPE_GPU, cps);
		devices = context.getInfo<CL_CONTEXT_DEVICES>();
		if (devices.size() <= deviceId)
		{
			throw Error(CL_INVALID_CONTEXT, "No compatible devices found!");
		}
	}
	catch (cl::Error error)
	{
		oclPrintError(error);
		throw error;
	}

	//Construct instance
	this->platformId = platformId;
	this->deviceId = deviceId;
	this->platform = platforms[platformId];
	this->device = devices[deviceId];
	this->context = context;
	this->queue = CommandQueue(context, devices[deviceId], CL_QUEUE_PROFILING_ENABLE);

	std::cout << "Device Attached: (" << platformId << ":" << deviceId << ") " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
	return true;
}
std::string GPUController::compileKernel(const std::string fileName, const std::string kernelName, const std::string params)
{
	try
	{
		//Read source code
		std::ifstream sourceFile(fileName.c_str());
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
		sourceFile.close();

		//Attach sha256 if included
		std::string baseInclude = "#include \"sha256.cl\"";
		size_t includePosition = sourceCode.find(baseInclude);
		if (includePosition != std::string::npos)
		{
			std::ifstream baseFile("sha256.cl");
			std::string baseCode(std::istreambuf_iterator<char>(baseFile), (std::istreambuf_iterator<char>()));
			baseFile.close();

			sourceCode.replace(includePosition, baseInclude.length(), baseCode);
		}
		
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

		//Compile
		std::vector<cl::Device> devices;
		devices.push_back(this->device);
		this->program = Program(this->context, source);
		this->program.build(devices, params.c_str());
		this->kernel = Kernel(this->program, kernelName.c_str());
		return std::string("");
	}
	catch (Error error)
	{
		oclPrintError(error);
		std::string buildlog;
		if (error.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			// Check the build status
			cl_build_status status = this->program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(this->device);

			// Get the build log
			std::string name = this->device.getInfo<CL_DEVICE_NAME>();
			buildlog = this->program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(this->device);
			std::cerr << "Build log for " << name << ":" << std::endl
				<< buildlog << std::endl;
		}
		return std::string(buildlog);
	}
	return std::string("");
}

//Utilities
void GPUController::hexToDec(const std::string hex, cl_uint* dec) const
{
	std::stringstream ss;
	#pragma unroll
	for (size_t i = 0; i < 8; i++)
	{
		ss << std::hex << hex.substr(i * 8, 8);
		ss >> dec[i];
		ss.str("");
		ss.clear();
	}
}