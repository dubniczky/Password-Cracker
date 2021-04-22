#include "GPUController.hpp"

#define _CRT_SECURE_NO_WARNINGS

unsigned int GPUController::hashMultiple(const std::string& infileName, const std::string& outfileName)
{
	//Print hash 
	const char* infilec = infileName.c_str();
	printf("Input file: %s\n", infilec);
	const char* outfilec = outfileName.c_str();
	printf("Output file: %s\n", outfilec);

	//Begin Hashing
	try
	{
		//Compile kernel
		printf("Compiling kernel...\n");
		if (compileKernel("hash_multiple.kernel.cl", "sha256hash_multiple_kernel") != "")
		{
			return 0;
		}
		printf("Kernel compiled.\n");


		//Open file
		FILE* infile = fopen(infilec, "r");
		if (!infile)
		{
			printf("Input file could not be opened.\n");
			return 0;
		}

		//Open file
		FILE* outfile = fopen(outfilec, "w+");
		if (!outfile)
		{
			printf("Output file could not be opened.\n");
			return 0;
		}


		//Initialize local variables
		printf("Initializing kernel...\n");
		unsigned int hashThreadCount = threadSize;
		cl::Event event;
		unsigned int lineCount = 0;
		unsigned int i = 0;
		bool bufferid = 0;
		bool run = true;


		//Generate buffer
		char* inputBuffers[2];
		inputBuffers[0] = new char[MAX_KEY_SIZE * hashThreadCount];
		inputBuffers[1] = new char[MAX_KEY_SIZE * hashThreadCount];
		char* currentBuffer = inputBuffers[0];

		char* result = new char[HASH_CHAR_SIZE * hashThreadCount + 1];
		result[HASH_CHAR_SIZE * hashThreadCount] = 0;

		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, MAX_KEY_SIZE * hashThreadCount);
		Buffer resultBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_CHAR_SIZE * hashThreadCount + 1);


		// Set arguments to kernel
		kernel.setArg(0, (cl_uint)MAX_KEY_SIZE);
		kernel.setArg(1, keyBuffer);
		kernel.setArg(2, resultBuffer);


		//Start timer
		printf("Hashing...\n");
		auto startTime = high_resolution_clock::now();

		
		for (; i < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * (size_t)i], MAX_KEY_SIZE, infile) != NULL; i++)
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


			//Wait kernel
			event.wait();
			queue.enqueueReadBuffer(resultBuffer, CL_TRUE, 0, HASH_CHAR_SIZE * i, result, NULL);


			//Write result
			result[i * HASH_CHAR_SIZE] = 0;
			fputs(result, outfile);


			//Prepare next
			lineCount += i;
			i = cline;
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<microseconds>(stopTime - startTime);

		fclose(infile);
		fclose(outfile);

		printf("Hash kernel finished.\n");

		long long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		//Cleanup
		delete[] inputBuffers[0];
		delete[] inputBuffers[1];
		delete[] result;

		return lineCount;
	}
	catch (Error error)
	{
		oclPrintError(error);
		return 0;
	}
}