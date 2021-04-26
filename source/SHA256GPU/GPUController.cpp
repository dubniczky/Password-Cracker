#include "GPUController.hpp"

//Construct
GPUController::GPUController()
{
	
}
GPUController::GPUController(const KernelProperties& props)
{
	this->attachDevice(props);
}

//Initialize
bool GPUController::attachDevice(const KernelProperties& props)
{
	//Guard
	if (!props.valid())
	{
		std::cout << "Invalid Properties Received on Device Attach" << std::endl;
		return false;
	}


	//Create context
	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;
	Context context;


	//Validate and initialize requested device
	try
	{
		//Get platform
		Platform::get(&platforms);
		if (platforms.size() <= props.platformId)
		{
			std::cout << "Invalid platform." << std::endl;
			return false;
		}
		cl_context_properties cps[3] =
		{
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[props.platformId])(),
			(cl_context_properties)0
		};


		//Get device
		context = Context(CL_DEVICE_TYPE_GPU, cps);
		devices = context.getInfo<CL_CONTEXT_DEVICES>();
		if (devices.size() <= props.deviceId)
		{
			std::cout << "Invalid device." << std::endl;
			return false;
		}
	}
	catch (cl::Error error)
	{
		oclPrintError(error);
		return false;
	}
	catch (...)
	{
		std::cout << "Selected device could not load." << std::endl;
		return false;
	}

	//Construct instance
	this->platformId = props.platformId;
	this->deviceId = props.deviceId;
	this->platform = platforms[props.platformId];
	this->device = devices[props.deviceId];
	this->threadSize = props.threadSize;

	this->context = context;	
	this->queue = CommandQueue(context, devices[props.deviceId], CL_QUEUE_PROFILING_ENABLE);

	std::cout << "Device Attached: (" << props.platformId << ":" << props.deviceId << ") " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
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
bool GPUController::hexToDec(const std::string hex, cl_uint* dec) const
{
	//Validate
	auto isNumber = [](char hx) { return hx >= 0x30 && hx <= 0x39; };
	auto isHexchar = [](char hx) { return hx >= 0x41 && hx <= 0x46 || hx >= 0x61 && hx <= 0x66; };

	int l = hex.length();
	if (l != 64) return false;

	for (int i = 0; i < 64; i++)
	{
		char c = hex[i];		
		if (!isNumber(c) && !isHexchar(c))
		{
			return false;
		}
	}


	//Convert
	std::stringstream ss;
	for (int i = 0; i < 8; i++)
	{
		ss << std::hex << hex.substr(i * 8, 8);
		ss >> dec[i];
		ss.str("");
		ss.clear();
	}
	return true;
}