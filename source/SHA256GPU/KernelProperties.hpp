#pragma once

#include "ArgList.hpp"

#define DEFAULT_PLATFORM 0
#define DEFAULT_DEVICE 0
#define DEFAULT_THREAD_SIZE 1024
#define DEFAULT_MAX_KEY_SIZE 25

#define MIN_THREAD_SIZE 1
#define MIN_MAX_KEY_SIZE 2
#define MAX_MAX_KEY_SIZE 55

#define PROPERTY_PLATFORM "-p"
#define PROPERTY_DEVICE_C "-d"
#define PROPERTY_THREAD_S "-t"
#define PROPERTY_KEY_SIZE "-k"
#define PROPERTY_ERROR "-"


struct KernelProperties
{
public:
	int platformId;
	int deviceId;
	int threadSize;
	int maxKeySize;

	/**
	  * Creates and instance of KernelProperties with the following arguments
	  * @param platformId The id of the platform.
	  * @param deviceId The id of the device.
	  * @param threadSize The number of keys in a thread at once.
	  * @param maxKeySize The maximum size of a key.
	 */
	KernelProperties(int platformId, int deviceId, int threadSize, int maxKeySize)
		: platformId(platformId), deviceId(deviceId), threadSize(threadSize), maxKeySize(maxKeySize)
	{

	}
	/**
	  * Creates and instance of KernelProperties with the default properties
	 */
	KernelProperties()
		: platformId(DEFAULT_PLATFORM),
		  deviceId(DEFAULT_DEVICE),
		  threadSize(DEFAULT_THREAD_SIZE),
		  maxKeySize(DEFAULT_MAX_KEY_SIZE)
	{

	}


	/**
	  * Determined if the current parameters are valid numbers in the accepted ranges (attaching device might fail despite).
	  * @return Are the parameters valid.
	 */
	bool valid() const
	{
		return this->platformId >= 0 &&
			   this->deviceId >= 0 &&
			   this->threadSize >= MIN_THREAD_SIZE &&
			   this->maxKeySize >= MIN_MAX_KEY_SIZE && this->maxKeySize <= MAX_MAX_KEY_SIZE; //TODO upper limit
	}

	/**
	  * Prints the properties formatted to the standard output.
	 */
	void print() const
	{
		std::cout << "Properties:" << std::endl
			<< "   Device: " << platformId << ":" << deviceId << std::endl
			<< "   Thread size: " << threadSize << std::endl
			<< "   Max key size: " << maxKeySize << std::endl;
	}

	//Static
	/**
	  * Create an arg list by fetching them from an ArgList object.
	  * @param properties object for the output results.
	  * @param args object for the fetching (fetched items are removed).
	  * @return Returns false if there was and error while parsing one or more arguments.
	 */
	static bool FromArgList(KernelProperties& properties, ArgList& args)
	{
		//Get properties
		std::string platform = args.fetchProperty(PROPERTY_PLATFORM);
		std::string device   = args.fetchProperty(PROPERTY_DEVICE_C);
		std::string threads  = args.fetchProperty(PROPERTY_THREAD_S);
		std::string keysize  = args.fetchProperty(PROPERTY_KEY_SIZE);

		if (platform == PROPERTY_ERROR || device == PROPERTY_ERROR ||
			threads == PROPERTY_ERROR || keysize == PROPERTY_ERROR)
		{
			return false;
		}

		//Platform
		if (isIntString(platform))
		{
			properties.platformId = std::stoi(platform);
		}

		//Device
		if (isIntString(device))
		{
			properties.deviceId = std::stoi(device);
		}

		//Thread Size
		if (isIntString(threads))
		{
			properties.threadSize = std::stoi(threads);
		}

		//Max Key Size
		if (isIntString(keysize))
		{
			properties.maxKeySize = std::stoi(keysize) + 1;
		}

		return true;
	}

private:
	/**
	  * Returns wether the string is a valid integer or not.
	  * @param str String to check.
	  * @return Returns true if the str is a valid integer.
	 */
	static bool isIntString(std::string str)
	{
		if (str.length() == 0) return false;

		for (size_t i = 0; i < str.length(); i++)
		{
			if (str[i] < 48 || str[i] > 57)
			{
				return false;
			}
		}
		return true;
	}
};