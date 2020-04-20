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

		//Make GPU buffers
		printf("Initializing kernel...\n");
		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, HASH_CHAR_SIZE * hashThreadCount);
		Buffer hashBuffer = Buffer(context, CL_MEM_READ_ONLY, MAX_KEY_SIZE * hashThreadCount);
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

		//Upload hash to gpu buffer sync
		queue.enqueueWriteBuffer(hashBuffer, CL_TRUE, 0, HASH_UINT_SIZE, hashDec);

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
				bufferid = false;
				currentBuffer = inputBuffer2;
			}

			// Set arguments to kernel
			kernel.setArg(0, MAX_KEY_SIZE);
			kernel.setArg(1, keyBuffer);
			kernel.setArg(2, hashBuffer);
			kernel.setArg(3, resultBuffer);

			//Run kernel
			NDRange _global_(i);
			lineCount += i;
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange, NULL, &eventQueue[0]);
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

		if (match == -1)
		{
			printf("===============\nNo match found.\n");

			printf("Lines verified: %d\n", lineCount);

			printf("===============\n");
		}
		else
		{
			printf("===============\nMatch found.\n");

			if (bufferid)
			{
				char* res = &inputBuffer1[hashThreadCount * MAX_KEY_SIZE];
				printf("Key: '%s'\n", res);
			}
			else
			{
				char* res = &inputBuffer2[hashThreadCount * MAX_KEY_SIZE];
				printf("Key: '%s'\n", res);
			}		
			
			printf("Line: %d\n", lineCount);

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