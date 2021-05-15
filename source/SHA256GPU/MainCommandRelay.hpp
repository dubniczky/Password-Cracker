#pragma once

#include <vector>
#include <string>

#include "GPUController.hpp"
#include "ArgList.hpp"

#define VERSION "v1.0"

#define DESC_PLATFORM printf("platform                                    : list available platforms and devices\n")
#define DESC_HASHSING printf("hash single <password>                      : hash a single password\n")
#define DESC_HASHSINS printf("hash single <password> <salt>               : hash a single password with salt\n")
#define DESC_HASHMULT printf("hash multiple <source> <target>             : hash multiple passwords from source to target file\n")
#define DESC_CRACSING printf("crack single <passwords> <hash>             : crack hash using a passwords source with options\n")

/**
 * Results given by the MainCommandRelay.
 */
enum class RelayResult
{
	//Command relayed successfully.
	RSuccess = 0,
	//Command has missing parameters.
	RIncomplete = 1,
	//Command has an unknown segment.
	RUnknown = 2,
	//Command has too many arguments.
	RLong = 3,
	//Command has errors in the properties.
	RUnresolved = 4,
	//Command has failed to attach the device to the controller.
	RAttach = 5
};

static class MainCommandRelay
{
public:
	//Level 0 relay
	/**
	 * Deconstruct the given command and run the applicable appication.
	 * 
	 * @param argc Argument count.
	 * @param args Argument array.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult relay(int argc, char* argv[])
	{
		//Convert args
		ArgList args;
		for (int i = 1; i < argc; i++)
		{
			args.add(std::string(argv[i]));
		}

		//Extract kernel properties
		KernelProperties props;
		if (!KernelProperties::FromArgList(props, args))
		{
			return RelayResult::RUnresolved;
		}


		if (args.count() == 0)
		{
			printf("SHA256GPU Cracker (%s) Commands:\n", VERSION);
			DESC_PLATFORM;
			DESC_HASHSING;
			DESC_HASHSINS;
			DESC_HASHMULT;
			DESC_CRACSING;

			printf("Properties: -p <id>      (default: 0) platform identifier\n");
			printf("            -d <id>      (default: 0) device identifier\n");
			printf("            -t <count>   (default: 1024) keys cracked at once\n");
			printf("            -k <size>    (default: 24) max key size\n");

			return RelayResult::RIncomplete;
		}

		if (args.first("platform"))
		{
			args.pop();
			return platformRelay(args, props);
		}
		else if (args.first("hash"))
		{
			args.pop();
			return hashRelay(args, props);
		}
		else if (args.first("crack"))
		{
			args.pop();
			return crackRelay(args, props);
		}
		
		printf("Unknown command: %s\n", args.get().c_str());
		printf("Use platform, hash or crack instead.\n");
		return RelayResult::RUnknown;
	}

private:
	//Level 1 relays
	/**
	 * Deconstruct the next layer of the given platform command and relay if resolved successfully. Called by a level 0 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult platformRelay(ArgList& args, const KernelProperties& properties)
	{
		GPUController* gpuc = new GPUController();
		gpuc->platformDetails();
		delete gpuc;

		return RelayResult::RSuccess;
	}

	/**
	 * Deconstruct the next layer of the given hash command and relay if resolved successfully. Called by a level 0 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult hashRelay(ArgList& args, const KernelProperties& properties)
	{
		//Incomplete
		if (args.count() == 0)
		{
			printf("Command incomplete.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			DESC_HASHMULT;
			return RelayResult::RIncomplete;
		}

		//Type
		if (args.first("single"))
		{
			args.pop();
			return hashSingleRelay(args, properties);
		}
		else if (args.first("multiple"))
		{
			args.pop();
			return hashMultipleRelay(args, properties);
		}

		//Unknown
		printf("Unknown command: %s\n", args.get().c_str());
		printf("Use single, or multiple instead.\n");
		return RelayResult::RUnknown;
	}

	/**
	 * Deconstruct the next layer of the given crack command and relay if resolved successfully. Called by a level 0 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult crackRelay(ArgList& args, const KernelProperties& properties)
	{
		//Incomplete
		if (args.count() == 0)
		{
			printf("Command incomplete.\n");
			DESC_CRACSING;
			return RelayResult::RIncomplete;
		}

		//Type
		if (args.first("single"))
		{
			args.pop();
			return crackSingleRelay(args, properties);
		}

		//Unknown
		printf("Unknown command: %s\n", args.get().c_str());
		printf("Use single instead.\n");
		return RelayResult::RUnknown;
	}

	//Level 2 relays
	/**
	 * Deconstruct the next layer of the given hash single command and relay if resolved successfully. Called by a level 1 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult hashSingleRelay(ArgList& args, const KernelProperties& properties)
	{
		//Validate
		int count = args.count();
		if (count == 0)
		{
			printf("Command incomplete.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			return RelayResult::RIncomplete;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			return RelayResult::RLong;
		}

		//Get parameters
		std::vector<std::string> arg;
		args.dump(arg);

		//Create
		GPUController* gpuc = new GPUController();
		if (!gpuc->attachDevice(properties))
		{
			return RelayResult::RAttach;
		}

		if (count == 1) //simple
		{
			gpuc->hashSingle(arg[0]);
		}
		else //salted
		{
			gpuc->hashSingleSalted(arg[0], arg[1]);
		}

		delete gpuc;
		return RelayResult::RSuccess;
	}

	/**
	 * Deconstruct the next layer of the given hash multiple command and relay if resolved successfully. Called by a level 1 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult hashMultipleRelay(ArgList& args, const KernelProperties& properties)
	{
		//Validate
		int count = args.count();
		if (count < 1)
		{
			printf("Command incomplete.\n");
			DESC_HASHMULT;
			return RelayResult::RIncomplete;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_HASHMULT;
			return RelayResult::RLong;
		}

		//Get parameters
		std::vector<std::string> arg;
		args.dump(arg);

		//Create
		GPUController* gpuc = new GPUController();
		if (!gpuc->attachDevice(properties))
		{
			return RelayResult::RAttach;
		}

		//Run
		gpuc->hashMultiple(arg[0], arg[1]);

		//Return
		delete gpuc;
		return RelayResult::RSuccess;
	}

	/**
	 * Deconstruct the next layer of the given crack single command and relay if resolved successfully. Called by a level 1 relay.
	 *
	 * @param args Argument List.
	 * @param properties KernelProperties object to attach with.
	 * @return RelayResult object depending on the outcome.
	 */
	static RelayResult crackSingleRelay(ArgList& args, const KernelProperties& properties)
	{
		//Validate
		int count = args.count();
		if (count == 0)
		{
			printf("Command incomplete.\n");
			DESC_CRACSING;
			return RelayResult::RIncomplete;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_CRACSING;
			return RelayResult::RLong;
		}

		//Get parameters
		std::vector<std::string> arg;
		args.dump(arg);

		//Create
		GPUController* gpuc = new GPUController();
		if (!gpuc->attachDevice(properties))
		{
			return RelayResult::RAttach;
		}

		//Run
		gpuc->crackSingle(arg[0], arg[1]);

		//Return
		delete gpuc;
		return RelayResult::RSuccess;
	}
};

