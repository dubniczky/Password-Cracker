#include "GPUController.hpp"

#define _CRT_SECURE_NO_WARNINGS

unsigned int GPUController::hashMultiple(std::string infileName, std::string outfileName)
{
	int hashThreadCount = threadSize;
	try
	{
		printf("Compiling kernel...\n");
		if (compileKernel("hash_multiple.kernel.cl", "sha256hash_multiple_kernel") != "")
		{
			return 0;
		}
		printf("Kernel compiled.\n");

		//Read keys
		printf("Reading file...\n");
		std::vector<std::string> lines;
		std::string line;
		std::ifstream infile(infileName);
		while (std::getline(infile, line))
		{
			lines.push_back(line);
		}
		infile.close();
		printf("Read %d lines.\n", lines.size());


		int iterations = (lines.size() / hashThreadCount) + (lines.size() % hashThreadCount > 0 ? 1 : 0);

		printf("Starting hash kernel...\n");
		std::ofstream outFile(outfileName.c_str());
		auto startTime = high_resolution_clock::now();

		cl_uint longest = 0;
		for (int i = 0; i < lines.size(); i++)
		{
			if (lines[i].length() > longest)
			{
				longest = lines[i].length();
			}
		}
		printf("Longest Key: %u\n", longest);
		longest += 1;
		cl_uint bufferSize = hashThreadCount * longest;

		Buffer keyBuffer = Buffer(context, CL_MEM_READ_ONLY, bufferSize);
		Buffer hashOutBuffer = Buffer(context, CL_MEM_WRITE_ONLY, HASH_CHAR_SIZE * hashThreadCount);
		char* result = new char[HASH_CHAR_SIZE * hashThreadCount];

		for (int i = 0; i < iterations; i++)
		{
			int start = i * hashThreadCount;
			if (i == iterations - 1 && lines.size() % hashThreadCount > 0)
			{
				hashThreadCount = lines.size() % hashThreadCount;
			}

			//Prepare buffer
			char* strBuffer = new char[bufferSize];
			for (int j = 0; j < hashThreadCount; j++)
			{
				strcpy(&strBuffer[j * longest], lines[start + j].c_str());
			}

			// Write data on input buffers!
			queue.enqueueWriteBuffer(keyBuffer, CL_TRUE, 0, bufferSize, strBuffer);

			// Set arguments to kernel
			kernel.setArg(0, longest);
			kernel.setArg(1, keyBuffer);
			kernel.setArg(2, hashOutBuffer);

			//Run kernel
			NDRange _global_(hashThreadCount);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, _global_, cl::NullRange);
			queue.enqueueReadBuffer(hashOutBuffer, CL_TRUE, 0, HASH_CHAR_SIZE * hashThreadCount, result);
			result[HASH_CHAR_SIZE * hashThreadCount - 1] = 0;

			//Write to file			
			outFile << result << std::endl;
			//printf("%s\n\n\n", result);
		}

		auto stopTime = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stopTime - startTime);

		printf("Kernel completed.\n");
		long micro = (long)duration.count();
		printf("Runtime: %lu microseconds.\n", micro);
		printf("         %f seconds.\n", micro / 1000000.0f);

		printf("Saving result to file...\n");
		outFile.close();
		delete[] result;
		printf("File saved.\n");
		return lines.size();
	}
	catch (Error error)
	{
		oclPrintError(error);
		return 0;
	}
}