#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "CppUnitTest.h"
#include "../SHA256GPU/GPUController.cpp"
#include "../SHA256GPU/hashSingle.cpp"
#include "../SHA256GPU/hashMultiple.cpp"
#include "../SHA256GPU/hashSingleSalted.cpp"
#include "../SHA256GPU/crackSingle.cpp"
#include "../SHA256GPU/crackSingleSalted.cpp"
#include "../SHA256GPU/platformDetails.cpp"

#include <process.h>

#pragma comment(lib, "OpenCL.lib")

#define KERNEL_DIR "../../../SHA256GPU/"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SHA256GPUTest
{
	TEST_CLASS(GPUControllerTest)
	{
	public:
		
		TEST_METHOD(CopyKernels)
		{
			auto copy = [](std::string name)
			{
				std::filesystem::remove(name);
				return std::filesystem::copy_file(KERNEL_DIR + name, name);
			};

			Assert::IsTrue(copy("sha256.cl"));
			Assert::IsTrue(copy("hash_single.kernel.cl"));
			Assert::IsTrue(copy("hash_multiple.kernel.cl"));
			Assert::IsTrue(copy("crack_single.kernel.cl"));
			Assert::IsTrue(copy("crack_single_salted.kernel.cl"));
		}

		TEST_METHOD(CreatingController)
		{
			GPUController* gc = new GPUController();
			Assert::IsNotNull(gc);
			Assert::IsTrue(gc->attachDevice());
		}

		TEST_METHOD(HashSingle)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice();
			
			//Hash
			Assert::AreEqual(std::string("b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e"), gc->hashSingle("banana"));
			Assert::AreEqual(std::string("c79c99dded78b97103916e94e5bc052d0b881ad2da896674b177bda1b1830e35"), gc->hashSingle("encloses"));
			Assert::AreEqual(std::string("9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2"), gc->hashSingle("enclosesHf45DD"));
			Assert::AreEqual(std::string("59557cf1890bf0b7458c1e66119ab01c3a796fd09df296ef7e70745d29934777"), gc->hashSingle("ex-wethouder"));
		}

		TEST_METHOD(HashSingleSalted)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice();

			//Hash
			Assert::AreEqual(std::string("9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2"), gc->hashSingleSalted("encloses", "Hf45DD"));
			Assert::AreEqual(std::string("1503052662d44b5f4aecd520103e84da17d1bf45579ade29abddc483efed07b1"), gc->hashSingleSalted("banana", "fc9h6fsd"));
		}

		TEST_METHOD(CrackSingle)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice();

			//Crack
			std::string pass100k = "../../../../passwords/passwords-100k.txt";
			Assert::AreEqual(std::string("banana"),
				             gc->crackSingle(pass100k, "b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e"));
			Assert::AreEqual(std::string("encloses"),
							 gc->crackSingle(pass100k, "c79c99dded78b97103916e94e5bc052d0b881ad2da896674b177bda1b1830e35"));
			Assert::AreEqual(std::string("12345"),
						 	 gc->crackSingle(pass100k, "5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5"));

			//Missing from passwords list (vizslakutya) hash
			Assert::AreEqual(std::string(""),
							 gc->crackSingle(pass100k, "1b72955fcf8258e459e16961bde674e7a364b4d85bce939eea774dd7250de06a"));

			//File not found
			Assert::AreEqual(std::string(""),
				gc->crackSingle("nonexistent.txt", "1b72955fcf8258e459e16961bde674e7a364b4d85bce939eea774dd7250de06a"));
		}
	};
}
