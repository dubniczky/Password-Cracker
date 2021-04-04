#include "GPUController.hpp"

std::string GPUController::crackSingleSalted(std::string infileName, std::string hash)
{
	//Write hash
	printf("Input: %s\n", hash.c_str());

	//Calc salt
	int saltLength = hash.length() - 64;
	printf("Salt length: %d\n", saltLength);
	char* salt = new char[saltLength + 1];
	memcpy(salt, hash.c_str(), saltLength);
	salt[saltLength] = 0;
	printf("Salt: %s\n", salt);

	char hashc[65];
	memcpy(hashc, hash.c_str() + saltLength, 65);

	//Write hexform
	printf("Hash hexform: [ ");
	#pragma unroll
	for (int i = 0; i < HASH_UINT_COUNT; i++)
	{
		printf("'%.8s', ", &hashc[i * 8]);
		if (i == HASH_UINT_COUNT - 1) printf("'%.8s' ", &hashc[i * 8]);
	}
	printf("]\n");

	//Write Decform
	cl_uint hashDec[8];
	hexToDec(hash, hashDec);
	printf("Hash decform: [ ");
	#pragma unroll
	for (int i = 0; i < HASH_UINT_COUNT; i++)
	{
		printf("%u, ", hashDec[i]);
		if (i == HASH_UINT_COUNT - 1) printf("%u ", hashDec[i]);
	}
	printf("]\n");

	char preproc[512];
	sprintf(preproc,
		    "-D HASH_0=%u -D HASH_1=%u -D HASH_2=%u -D HASH_3=%u -D HASH_4=%u -D HASH_5=%u -D HASH_6=%u -D HASH_7=%u \
             -D KEY_LENGTH=%d -D SALT_LENGTH=%d -D SALT_STRING=\"%s\"",
		    hashDec[0], hashDec[1], hashDec[2], hashDec[3], hashDec[4], hashDec[5], hashDec[6], hashDec[7],
			MAX_KEY_SIZE, saltLength, salt);

	//Calculate optimal thread size
	cl_ulong maxMemoryAlloc = device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
	cl_ulong unitSize = MAX_KEY_SIZE;
	int hashThreadCount = min(maxMemoryAlloc / MAX_KEY_SIZE, 46960);

	printf("Optimal thread size: %u\n", hashThreadCount);
	
	try
	{
		printf("Compiling kernel...\n");
		if (!compileKernel("crack_single_salted.kernel.cl", "sha256crack_single_salted_kernel", preproc))
		{
			return std::string();
		}
		printf("Kernel compiled.\n");

		//Make GPU buffers
		printf("Initializing kernel...\n");
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, HASH_CHAR_SIZE * hashThreadCount);
		Buffer resultBuffer = Buffer(context, CL_MEM_WRITE_ONLY, hashThreadCount);

		//Initialize local variables
		char* result = new char[hashThreadCount];
		char* inputBuffer1 = new char[MAX_KEY_SIZE * hashThreadCount];
		char* inputBuffer2 = new char[MAX_KEY_SIZE * hashThreadCount];
		char* currentBuffer = inputBuffer1;
		size_t match = -1;
		int lineCount = 0;
		cl::vector<Event> eventQueue;
		int i = 0;
		int previ = 0;
		char bufferid = 0;
		bool finished = false;

		//Open file
		FILE* infile = fopen(infileName.c_str(), "r");
		if (!infile)
		{
			printf("Infile could not be opened.\n");
		}

		//Start timer
		auto startTime = high_resolution_clock::now();

		printf("Cracking...\n");
		for (; i < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * i], MAX_KEY_SIZE, infile) != NULL; i++)
		{

		}

		while (true)
		{
			if (finished)
			{
				break;
			}

			// Write data on input buffers!
			queue.enqueueWriteBuffer(keyBuffer, CL_FALSE, 0, MAX_KEY_SIZE * i, currentBuffer, &eventQueue);
			if (bufferid)
			{
				bufferid = false;
				currentBuffer = inputBuffer1;
			}
			else
			{
				bufferid = true;
				currentBuffer = inputBuffer2;
			}

			// Set arguments to kernel
			kernel.setArg(0, keyBuffer);
			kernel.setArg(1, resultBuffer);

			//Run kernel
			NDRange globalRange(i);
			lineCount += i;
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, cl::NullRange, NULL, &eventQueue[0]);
			queue.enqueueReadBuffer(resultBuffer, CL_FALSE, 0, i, result, &eventQueue);

			//Read lines
			int cline = 0;
			for (; cline < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * cline], MAX_KEY_SIZE, infile); cline++)
			{

			}
			if (cline == 0) finished = true;

			//Await kernel
			eventQueue[0].wait();

			//Verify match
			for (int j = 0; j < i; j++)
			{
				if (result[j])
				{
					match = lineCount + j;
					hashThreadCount = j;
					break;
				}
			}

			i = cline;

			if (match != -1)
			{
				break;
			}
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stopTime - startTime);

		fclose(infile);

		printf("Crack kernel completed.\n");

		std::string outString;
		if (match == -1)
		{
			printf("===============\nNo match found.\n");

			printf("Lines verified: %d\n", lineCount);
			outString = "";

			printf("===============\n");
		}
		else
		{
			printf("===============\nMatch found.\n");

			if (bufferid)
			{
				char* res = &inputBuffer1[hashThreadCount * MAX_KEY_SIZE];
				printf("Key: '%s'\n", res);
				outString = std::string(res);
			}
			else
			{
				char* res = &inputBuffer2[hashThreadCount * MAX_KEY_SIZE];
				printf("Key: '%s'\n", res);
				outString = std::string(res);
			}

			printf("Line: %d\n", lineCount);

			printf("===============\n");
		}

		long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		delete[] result;
		delete[] salt;
		return outString;
	}
	catch (Error error)
	{
		oclPrintError(error);
		return std::string();
	}
}