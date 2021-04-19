﻿#include "GPUController.hpp"

std::string GPUController::crackSingleSalted(std::string infileName, std::string saltHash)
{
	//Separate salt and hash
	unsigned int saltLength = saltHash.length() - 64;
	std::string salt = saltHash.substr(0, saltLength);
	std::string hash = saltHash.substr(saltLength, 64);
	const char* csalt = salt.c_str();
	const char* chash = hash.c_str();


	//Write hash
	printf("Input: %s\n", hash.c_str());
	printf("Hash: %s\n", hash.c_str());
	printf("Salt: %s\n", salt.c_str());


	//Print hexadecimal form	
	printf("Hash hexform: [ ");
	for (int i = 0; i < HASH_UINT_COUNT - 1; i++)
	{
		printf("'%.8s', ", &chash[i * 8]);
	}
	printf("'%.8s' ]\n", &chash[(HASH_UINT_COUNT - 1) * 8]);


	//Print decimal form
	cl_uint hashDec[8];
	hexToDec(hash, hashDec);
	printf("Hash decform: [ ");
	for (int i = 0; i < HASH_UINT_COUNT - 1; i++)
	{
		printf("%u, ", hashDec[i]);
	}
	printf("%u ]\n", hashDec[HASH_UINT_COUNT - 1]);


	//Begin cracking
	try
	{
		//Generate compiler command
		char command[256];
		sprintf_s(command,
				  "-D HASH_0=%u -D HASH_1=%u -D HASH_2=%u -D HASH_3=%u -D HASH_4=%u "
				        "-D HASH_5=%u -D HASH_6=%u -D HASH_7=%u -D KEY_LENGTH=%d -D SALT_LENGTH=%d -D SALT_STRING=%s",
				  hashDec[0], hashDec[1], hashDec[2], hashDec[3], hashDec[4],
				  hashDec[5], hashDec[6], hashDec[7], MAX_KEY_SIZE, saltLength, csalt);


		//Compile kernel
		printf("Compiling kernel...\n");
		if (compileKernel("crack_single_salted.kernel.cl", "sha256crack_single_salted_kernel", command) != "")
		{
			return std::string("Error while compiling kernel.");
		}
		printf("Kernel compiled.\n");


		//Open file
		FILE* infile = fopen(infileName.c_str(), "r");
		if (!infile)
		{
			printf("File could not be opened.\n");
			return std::string("File could not be opened.");
		}


		//Initialize local variables
		printf("Initializing kernel...\n");
		unsigned int hashThreadCount = threadSize;
		unsigned int result = 0;
		cl::Event event;
		unsigned int lineCount = 0;
		unsigned int i = 0;
		unsigned int previ = 0;
		bool bufferid = 0;
		bool run = true;


		//Generate buffers	
		char* inputBuffers[2];
		inputBuffers[0] = new char[MAX_KEY_SIZE * threadSize];
		inputBuffers[1] = new char[MAX_KEY_SIZE * threadSize];
		char* currentBuffer = inputBuffers[0];

		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, HASH_CHAR_SIZE * threadSize);
		Buffer resultBuffer = Buffer(context, CL_MEM_ALLOC_HOST_PTR, sizeof(int));


		// Set arguments to kernel
		kernel.setArg(0, keyBuffer);
		kernel.setArg(1, resultBuffer);


		//Start timer
		auto startTime = high_resolution_clock::now();

		printf("Cracking...\n");
		for (; i < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * i], MAX_KEY_SIZE, infile) != NULL; i++)
		{

		}

		while (run)
		{
			queue.enqueueWriteBuffer(keyBuffer, CL_FALSE, 0, MAX_KEY_SIZE * i, currentBuffer, NULL);


			//Swap buffers
			bufferid = (bufferid + 1) % 2;
			currentBuffer = inputBuffers[bufferid];


			//Run kernel
			NDRange _global_(i);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange, NULL, &event);

			//Read lines
			unsigned int cline = 0;
			for (; cline < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * cline], MAX_KEY_SIZE, infile); cline++)
			{

			}
			if (cline == 0) run = false;


			//Await kernel
			event.wait();
			queue.enqueueReadBuffer(resultBuffer, CL_TRUE, 0, sizeof(int), &result, NULL);

			//Check match
			if (result > 0) //Step out: match
			{
				lineCount += result;
				run = false;
			}
			else //Step over: no match
			{
				lineCount += i;
				i = cline;
				//printf("%u\n", lineCount);
			}
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<microseconds>(stopTime - startTime);

		fclose(infile);

		printf("Crack kernel finished.\n");

		std::string outString = "";
		if (result == 0)
		{
			printf("===============\nNo match found.\n");
			printf("Lines verified: %d\n", lineCount);
			printf("===============\n");
		}
		else
		{
			//Get result 
			char* res = &inputBuffers[!bufferid][(result - 1) * MAX_KEY_SIZE];

			//Remove line break
			for (int i = 0; i < MAX_KEY_SIZE; i++)
			{
				if (res[i] == '\n')
				{
					res[i] = 0;
					break;
				}
			}

			outString = std::string(res);

			printf("===============\nMatch found.\n");
			printf("Key: '%s'\n", res);
			printf("Line: %d\n===============\n", lineCount);
		}

		long long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		//Cleanup
		delete[] inputBuffers[0];
		delete[] inputBuffers[1];

		return outString;
	}
	catch (Error error)
	{
		oclPrintError(error);
		return std::string();
	}
}