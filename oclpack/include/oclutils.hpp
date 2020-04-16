#pragma once

//Use cl::vector instead of STL version
//#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS

// All OpenCL headers
#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <iostream>
#include <iterator>
#include <algorithm>
#include <regex>
#include <string>
#include <fstream>

#pragma region Misc

inline std::string oclReadSourcesFromFile( const char *file_name )
{
	// Read source file
	std::ifstream sourceFile(file_name);
	std::string sourceCode(
		std::istreambuf_iterator<char>(sourceFile),
		(std::istreambuf_iterator<char>()));
	return sourceCode;
}

#pragma endregion

#pragma region Context Utils

inline bool oclCreateContextByRegex(cl::Context &context, std::regex platform_name = std::regex("nvidia", std::regex_constants::ECMAScript | std::regex_constants::icase), cl_device_type device_type = CL_DEVICE_TYPE_GPU)
{
#ifdef __NO_STD_VECTOR
	cl::vector<cl::Platform> platforms;
#else
	std::vector<cl::Platform> platforms;
#endif
	cl::Platform::get(&platforms);

	for (auto platform : platforms) {

		try {
			auto name = platform.getInfo<CL_PLATFORM_NAME>();

			if(!std::regex_search(name, platform_name))
				continue;

			cl_context_properties cps[3] = {
				CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
				0
			};

			context = cl::Context(device_type, cps);
			auto numDevices = context.getInfo<CL_CONTEXT_NUM_DEVICES>();

			if (numDevices > 0)
				return true;
		}
		catch (cl::Error error) {
			continue;
		}
	}

	return false;
}

inline bool oclCreateContextBy(cl::Context &context, std::string platform_name_substring = "", cl_device_type device_type = CL_DEVICE_TYPE_GPU)
{
	return oclCreateContextByRegex(context, std::regex(platform_name_substring, std::regex_constants::ECMAScript | std::regex_constants::icase), device_type);
}

#pragma endregion

#pragma region Alignment Utils

inline cl_uint oclZeroCopyPtrAlignment (cl::Device device)
{
	return device.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>();
}

inline cl_uint oclZeroCopySizeAlignment (cl_uint requiredSize, cl::Device device)
{
	// The following statement rounds requiredSize up to the next CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE-byte boundary
	return requiredSize + (~requiredSize + 1) % device.getInfo<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>();  
} 

#pragma endregion

#pragma region Timing Utils

inline cl_int oclGetTimeStats(cl_event event, cl_ulong &execStart, cl_ulong &execEnd)
{
	cl_int err = CL_SUCCESS;

	if(event == NULL) {
		std::cerr << "No event object returned!" << std::endl;
	} else {
		clWaitForEvents(1, &event);
	}

	err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &execStart, NULL);
	err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &execEnd, NULL);

	return err;
}

inline cl_int oclPrintTimeStats(cl_event event)
{
	cl_ulong execStart, execEnd;
	cl_int err = oclGetTimeStats(event, execStart, execEnd);

	std::cout << "[start] " << execStart <<
		" [end] "   << execEnd <<
		" [time] "  << (execEnd - execStart) / 1e+06 << "ms." << std::endl;

	return err;
}

inline void oclGetTimeStats(cl::Event operation, cl_ulong &device_start, cl_ulong &device_end)
{
	operation.wait();
	operation.getProfilingInfo<cl_ulong>(CL_PROFILING_COMMAND_START, &device_start);
	operation.getProfilingInfo<cl_ulong>(CL_PROFILING_COMMAND_END, &device_end);
}

inline double oclGetTiming(cl::Event operation)
{
	cl_ulong device_start, device_end;
	oclGetTimeStats(operation, device_start, device_end);
	return (device_end - device_start) / 1e+06;
}

#pragma endregion

#pragma region String Utils

inline const char* oclChannelOrderString(cl_channel_order order)
{
	switch (order)
	{
	case CL_R        : return "CL_R";
	case CL_A        : return "CL_A";
	case CL_RG       : return "CL_RG";
	case CL_RA       : return "CL_RA";
	case CL_RGB      : return "CL_RGB";
	case CL_RGBA     : return "CL_RGBA";
	case CL_BGRA     : return "CL_BGRA";
	case CL_ARGB     : return "CL_ARGB";
	case CL_INTENSITY: return "CL_INTENSITY";
	case CL_LUMINANCE: return "CL_LUMINANCE";
	case CL_Rx       : return "CL_Rx";
	case CL_RGx      : return "CL_RGx";
	case CL_RGBx     : return "CL_RGBx";
	default:
		return "error!";
	}
}

inline const char* oclChannelTypeString(cl_channel_type type)
{
	switch (type)
	{
	case CL_SNORM_INT8      : return "CL_SNORM_INT8";
	case CL_SNORM_INT16     : return "CL_SNORM_INT16";
	case CL_UNORM_INT8      : return "CL_UNORM_INT8";
	case CL_UNORM_INT16     : return "CL_UNORM_INT16";
	case CL_UNORM_SHORT_565 : return "CL_UNORM_SHORT_565";
	case CL_UNORM_SHORT_555 : return "CL_UNORM_SHORT_555";
	case CL_UNORM_INT_101010: return "CL_UNORM_INT_101010";
	case CL_SIGNED_INT8     : return "CL_SIGNED_INT8";
	case CL_SIGNED_INT16    : return "CL_SIGNED_INT16";
	case CL_SIGNED_INT32    : return "CL_SIGNED_INT32";
	case CL_UNSIGNED_INT8   : return "CL_UNSIGNED_INT8";
	case CL_UNSIGNED_INT16  : return "CL_UNSIGNED_INT16";
	case CL_UNSIGNED_INT32  : return "CL_UNSIGNED_INT32";
	case CL_HALF_FLOAT      : return "CL_HALF_FLOAT";
	case CL_FLOAT           : return "CL_FLOAT";
	default:
		return "error!";
	}
}

inline const char* oclDeviceTypeString(cl_int devType)
{
	switch (devType)
	{
	case CL_DEVICE_TYPE_DEFAULT: return "CL_DEVICE_TYPE_DEFAULT";
	case CL_DEVICE_TYPE_CPU: return "CL_DEVICE_TYPE_CPU";
	case CL_DEVICE_TYPE_GPU: return "CL_DEVICE_TYPE_GPU";
	case CL_DEVICE_TYPE_ACCELERATOR: return "CL_DEVICE_TYPE_ACCELERATOR";
	case CL_DEVICE_TYPE_ALL: return "CL_DEVICE_TYPE_ALL";
	default:
		return "error!";
	}       
}

// Helper function to get OpenCL error string from constant
// *********************************************************************
inline const char* oclErrorString(cl_int error)
{
	static const char* errorString[] = {
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE",
	};

	const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

	const int index = -error;

	return (index >= 0 && index < errorCount) ? errorString[index] : "Unspecified Error";
}

#pragma endregion

#pragma region Error Utils

inline void oclPrintError(const cl::Error &error)
{
	std::cout << error.what() << "(" << error.err() << " == " << oclErrorString(error.err()) << ")" << std::endl;
}

#pragma endregion