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

	KernelProperties(int platformId, int deviceId, int threadSize, int maxKeySize)
		: platformId(platformId), deviceId(deviceId), threadSize(threadSize), maxKeySize(maxKeySize)
	{

	}
	KernelProperties()
		: platformId(DEFAULT_PLATFORM),
		  deviceId(DEFAULT_DEVICE),
		  threadSize(DEFAULT_THREAD_SIZE),
		  maxKeySize(DEFAULT_MAX_KEY_SIZE)
	{

	}

	bool valid() const
	{
		return this->platformId >= 0 &&
			   this->deviceId >= 0 &&
			   this->threadSize >= MIN_THREAD_SIZE &&
			   this->maxKeySize >= MIN_MAX_KEY_SIZE && this->maxKeySize <= MAX_MAX_KEY_SIZE; //TODO upper limit
	}

	void print()
	{
		std::cout << "Properties:" << std::endl
			<< "   Device: " << platformId << ":" << deviceId << std::endl
			<< "   Thread size: " << threadSize << std::endl
			<< "   Max key size: " << maxKeySize << std::endl;
	}

	//Static
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