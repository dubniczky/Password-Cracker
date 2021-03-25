#include "GPUController.hpp"

void GPUController::platform()
{
	try
	{
		std::cout << "Use the given device ID of the graphics controller to specify the cracking device." << std::endl;

		// Get available platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

		std::cout << "Platforms: " << platforms.size() << std::endl << std::endl;

		vector<Device> devices;
		Context context;

		int platformID = 0;
		for (Platform p : platforms)
		{
			try
			{
				std::cout << "* " << p.getInfo<CL_PLATFORM_NAME>() << std::endl;
				std::cout << "\t" << p.getInfo<CL_PLATFORM_VERSION>() << std::endl;

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

				std::cout << "\tDevices: " << devices.size() << std::endl << std::endl;

				int deviceID = 0;
				for (const Device& device : devices)
				{
					std::cout << "\t* " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
					std::cout << "\t\tDevice ID:        (" << platformID << ":" << deviceID << ")" << std::endl;
					std::cout << "\t\tDevice available: " << ((device.getInfo<CL_DEVICE_AVAILABLE>()) ? "yes" : "no") << std::endl;
					std::cout << "\t\tClock frequency:  " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz" << std::endl;
					
					std::cout << "\t\tMemory size:      " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024.0f / 1024.0f << " MB" << std::endl;
					std::cout << "\t\t       alloc:     " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / 1024 / 1024 << " MB" << std::endl;
					std::cout << "\t\tCache type:       " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>() << std::endl;
					std::cout << "\t\t      size:       " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() / 1024.0f << " KB" << std::endl;

					std::cout << "\t\tVersion device:   " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;
					std::cout << "\t\t        driver:   " << device.getInfo<CL_DRIVER_VERSION>() << std::endl;
					deviceID++;
				}
			}
			catch (Error error)
			{
				oclPrintError(error);
			}
			platformID++;
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