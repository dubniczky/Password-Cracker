#include "GPUController.hpp"

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
					std::cout << "\tMemory size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024.0f / 1024.0f << " MB" << std::endl;
					std::cout << "\tCache type: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>() << std::endl;
					std::cout << "\tCache size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() / 1024.0f << " KB" << std::endl;
					std::cout << "\tCacheline size: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>() << std::endl;
					std::cout << "\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz" << std::endl;
					std::cout << "\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
					std::cout << "\tMax memory alloc size: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / 1024 / 1024 << " MB" << std::endl;
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