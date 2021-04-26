#include "GPUController.hpp"

std::string GPUController::platformDetails() const
{
	try
	{
		// Get available platforms
		std::vector<Platform> platforms;
		Platform::get(&platforms);

		if (platforms.size() == 0)
		{
			std::cout << "No device platforms detected." << std::endl << std::endl;
			return "No platforms.";
		}
		else
		{
			std::cout << "Use the given device ID of the graphics controller to specify the cracking device." << std::endl;
			std::cout << "Platforms: " << platforms.size() << std::endl << std::endl;
		}

		std::vector<Device> devices;
		Context context;
		int platformID = -1;


		for (Platform p : platforms)
		{
			try
			{
				platformID++;
				std::cout << "* " << p.getInfo<CL_PLATFORM_NAME>() << std::endl;
				std::cout << "\t" << p.getInfo<CL_PLATFORM_VERSION>() << std::endl;

				// Select the default platform and create a context using this platform and the GPU
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
					std::cout << "\t\tMemory alloc:     " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / 1024 / 1024 << " MB" << std::endl;
					std::cout << "\t\tCache type:       " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>() << std::endl;
					std::cout << "\t\tCache size:       " << device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() / 1024.0f << " KB" << std::endl;

					std::cout << "\t\tDevice Version:   " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;
					std::cout << "\t\tDriver Version:   " << device.getInfo<CL_DRIVER_VERSION>() << std::endl;
					deviceID++;
				}
			}
			catch (Error error)
			{
				std::cout << "Error while loading device." << std::endl;
				oclPrintError(error);
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
		return std::string(error.what());
	}

	return "";
}