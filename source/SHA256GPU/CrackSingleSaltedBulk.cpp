#include "GPUController.hpp"

void GPUController::crackSingleSaltedBulk(std::string infileName, std::string hash, unsigned int count)
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
	hexToDec(hashc, hashDec);
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
	cl_ulong maxMemoryAlloc = devices[deviceId].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
	cl_ulong unitSize = MAX_KEY_SIZE;
	int hashThreadCount = min(maxMemoryAlloc / MAX_KEY_SIZE, count);
	if (hashThreadCount != count)
	{
		printf("Bulk hash count reduced due to GPU resource limitations");
	}
	printf("Optimal thread size: %u\n", hashThreadCount);

	try
	{
		printf("Compiling kernel...\n");
		if (!compileKernel("crack_single_salted.kernel.cl", "sha256crack_single_salted_kernel", preproc))
		{
			return;
		}
		printf("Kernel compiled.\n");

		//Make GPU buffers
		printf("Initializing kernel...\n");
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, HASH_CHAR_SIZE * hashThreadCount);
		Buffer resultBuffer = Buffer(context, CL_MEM_WRITE_ONLY, hashThreadCount);

		//Initialize local variables
		char* result = new char[hashThreadCount];
		char* inputBuffer = new char[MAX_KEY_SIZE * hashThreadCount];
		cl::vector<Event> eventQueue;
		int i = 0;

		//Open file
		FILE* infile = fopen(infileName.c_str(), "r");
		if (!infile)
		{
			printf("Infile could not be opened.\n");
		}

		for (; i < hashThreadCount && fgets(&inputBuffer[MAX_KEY_SIZE * i], MAX_KEY_SIZE, infile) != NULL; i++)
		{

		}

		//Start timer
		auto startTime = high_resolution_clock::now();

		printf("Cracking...\n");
		// Write data on input buffers!
		queue.enqueueWriteBuffer(keyBuffer, CL_FALSE, 0, MAX_KEY_SIZE * i, inputBuffer, &eventQueue);

		// Set arguments to kernel
		kernel.setArg(0, keyBuffer);
		kernel.setArg(1, resultBuffer);

		//Run kernel
		NDRange globalRange(i);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, cl::NullRange, NULL, &eventQueue[0]);
		queue.enqueueReadBuffer(resultBuffer, CL_FALSE, 0, i, result, &eventQueue);

		//Await kernel
		eventQueue[0].wait();

		//Verify match
		int matchIndex = -1;
		for (int j = 0; j < i; j++)
		{
			if (result[j])
			{
				matchIndex = j;
				break;
			}
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stopTime - startTime);

		fclose(infile);

		printf("Crack kernel completed.\n");

		if (matchIndex == -1)
		{
			printf("===============\nNo match found.\n");

			printf("Lines verified: %d\n", i);

			printf("===============\n");
		}
		else
		{
			printf("===============\nMatch found.\n");

			char* res = &inputBuffer[matchIndex * MAX_KEY_SIZE];
			printf("Key: '%s'\n", res);

			printf("Line: %d\n", i);

			printf("===============\n");
		}

		long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		delete[] result;
		delete[] hashDec;
		delete[] salt;
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}