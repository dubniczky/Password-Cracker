#pragma once

#include <vector>
#include <string>

#include "GPUController.hpp"
#include "LinkedList.hpp"

#define DESC_PLATFORM printf("platform                                    : list available platforms and devices\n")
#define DESC_HASHSING printf("hash single <password>                      : hash a single password\n")
#define DESC_HASHSINS printf("hash single <password> <salt>               : hash a single password with salt\n")
#define DESC_HASHMULT printf("hash multiple <source> <target>             : hash multiple passwords from source to target file\n")
#define DESC_CRACSING printf("crack single <passwords> <hash>             : crack hash using a passwords source with options\n")

class MainCommandRelay
{
public:
	static int relay(int argc, char* argv[])
	{
		//Convert args
		LinkedList<std::string> args;
		for (int i = 1; i < argc; i++)
		{
			args.add(std::string(argv[i]));
		}

		if (args.count() == 0)
		{
			printf("SHA256GPU Cracker (v1.0) Commands:\n");
			DESC_PLATFORM;
			DESC_HASHSING;
			DESC_HASHSINS;
			DESC_HASHMULT;
			DESC_CRACSING;
			return 0;
		}

		if (args.first("platform"))
		{
			args.pop();
			return platformRelay(args);
		}
		else if (args.first("hash"))
		{
			args.pop();
			return hashRelay(args);
		}
		else if (args.first("crack"))
		{
			args.pop();
			return crackRelay(args);
		}
		else
		{
			printf("Unknown command: %s\n", args.get().c_str());
			printf("Use platform, hash or crack instead.\n");
			return 1;
		}
	}

private:
	//Level 1 relays
	static int platformRelay(LinkedList<std::string>& args)
	{
		GPUController* gpuc = new GPUController();
		gpuc->platformDetails();
		delete gpuc;
		return 0;
	}
	static int hashRelay(LinkedList<std::string>& args)
	{
		if (args.count() == 0)
		{
			printf("Command incomplete.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			DESC_HASHMULT;
			return 0;
		}

		if (args.first("single"))
		{
			args.pop();
			return hashSingleRelay(args);
		}
		else if (args.first("multiple"))
		{
			args.pop();
			return hashMultipleRelay(args);
		}
		else
		{
			printf("Unknown command: %s\n", args.get().c_str());
			printf("Use single, or multiple instead.\n");
			return 1;
		}

		return 0;
	}
	static int crackRelay(LinkedList<std::string>& args)
	{
		if (args.count() == 0)
		{
			printf("Command incomplete.\n");
			DESC_CRACSING;
			return 0;
		}

		if (args.first("single"))
		{
			args.pop();
			return crackSingleRelay(args);
		}
		else
		{
			printf("Unknown command: %s\n", args.get().c_str());
			printf("Use single instead.\n");
			return 1;
		}

		return 0;
	}

	//Level 2 relays
	static int hashSingleRelay(LinkedList<std::string>& args)
	{
		//Validate
		int count = args.count();
		if (count == 0)
		{
			printf("Command incomplete.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			return 0;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_HASHSING;
			DESC_HASHSINS;
			return 1;
		}

		//Run
		std::vector<std::string> arg;
		args.dump(arg);
		GPUController* gpuc = new GPUController();
		gpuc->attachDevice();
		
		if (count == 1) //simple
		{
			gpuc->hashSingle(arg[0]);
		}
		else //salted
		{
			gpuc->hashSingleSalted(arg[0], arg[1]);
		}

		delete gpuc;
		return 0;
	}
	static int hashMultipleRelay(LinkedList<std::string>& args)
	{
		//Validate
		int count = args.count();
		if (count < 1)
		{
			printf("Command incomplete.\n");
			DESC_HASHMULT;
			return 0;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_HASHMULT;
			return 1;
		}

		//Run
		std::vector<std::string> arg;
		args.dump(arg);

		auto gpuc = new GPUController();
		gpuc->attachDevice();
		gpuc->hashMultiple(arg[0], arg[1]);
		delete gpuc;
		return 0;
	}
	static int crackSingleRelay(LinkedList<std::string>& args)
	{
		//Validate
		int count = args.count();
		if (count == 0)
		{
			printf("Command incomplete.\n");
			DESC_CRACSING;
			return 0;
		}
		if (count > 2)
		{
			printf("Command has too many parameters.\n");
			DESC_CRACSING;
			return 1;
		}

		//Run
		std::vector<std::string> arg;
		args.dump(arg);
		GPUController* gpuc = new GPUController();
		gpuc->attachDevice();
		gpuc->crackSingle(arg[0], arg[1]);

		delete gpuc;
		return 0;
	}
};

