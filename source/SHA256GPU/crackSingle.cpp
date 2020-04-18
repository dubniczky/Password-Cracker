#include "GPUController.hpp"

void GPUController::crackSingle(string infileName, string hash)
{
	//Write hash
	const char* hashc = hash.c_str();
	printf("Hash: %s\n", hashc);

	//Write hexform
	#pragma unroll
	printf("Hash hexform: [ ");
	for (int i = 0; i < HASH_UINT_COUNT; i++)
	{
		printf("'%.8s', ", &hashc[i*8]);
		if (i == HASH_UINT_COUNT - 1) printf("'%.8s' ", &hashc[i * 8]);
	}
	printf("]\n");
	
	//Write Decform
	cl_uint* hashDec = hexdec(hashc);
	printf("Hash decform: [ ");
	#pragma unroll
	for (int i = 0; i < HASH_UINT_COUNT; i++)
	{
		printf("%u, ", hashDec[i]);
		if (i == HASH_UINT_COUNT - 1) printf("%u ", hashDec[i]);
	}
	printf("]\n");

	int hashThreadCount = HASH_THREAD_COUNT;
	try
	{
		printf("Compiling kernel...\n");
		if (!compileKernel("crack_single.kernel.cl", "sha256crack_single_kernel"))
		{
			return;
		}
		printf("Kernel compiled.\n");

		//Prepare input
		printf("Starting crack kernel...\n");
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, HASH_CHAR_SIZE * hashThreadCount);
		Buffer hashBuffer = Buffer(context, CL_MEM_READ_ONLY, MAX_KEY_SIZE * hashThreadCount);
		Buffer resultBuffer = Buffer(context, CL_MEM_WRITE_ONLY, hashThreadCount);

		char* result = new char[hashThreadCount];
		char* inputBuffer = new char[MAX_KEY_SIZE * hashThreadCount];

		std::ifstream infile(infileName.c_str());

		size_t match = -1;
		long long lineCount = 0;

		queue.enqueueWriteBuffer(hashBuffer, CL_TRUE, 0, HASH_UINT_SIZE, hashDec);
		auto startTime = high_resolution_clock::now();

		printf("Cracking...\n");
		while (!infile.eof())
		{
			//printf("%d\n", lineCount);
			std::string line;
			int i = 0;
			for (; i < hashThreadCount && std::getline(infile, line); i++)
			{
				strcpy(&inputBuffer[MAX_KEY_SIZE * i], line.c_str());
			}
			
			// Write data on input buffers!
			queue.enqueueWriteBuffer(keyBuffer, CL_TRUE, 0, MAX_KEY_SIZE * i, inputBuffer);

			// Set arguments to kernel
			kernel.setArg(0, MAX_KEY_SIZE);
			kernel.setArg(1, keyBuffer);
			kernel.setArg(2, hashBuffer);
			kernel.setArg(3, resultBuffer);

			//Run kernel
			NDRange _global_(i);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);
			queue.enqueueReadBuffer(resultBuffer, CL_TRUE, 0, i, result);

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
			if (match != -1)
			{
				break;
			}

			lineCount += i;
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stopTime - startTime);

		infile.close();

		printf("Crack kernel completed.\n");

		if (match == -1)
		{
			printf("===============\nNo match found.\n===============\n");
		}
		else
		{
			printf("===============\nMatch found.\n");

			char* res = &inputBuffer[hashThreadCount * MAX_KEY_SIZE];
			printf("Key: '%s'\n", res);
			printf("Line: %d\n", match+1);

			printf("===============\n");
		}
		
		long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		delete[] result;
		delete[] hashDec;
		//delete[] inputBuffer;
	}
	catch (Error error)
	{
		oclPrintError(error);
	}
}