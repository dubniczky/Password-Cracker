//Base imports
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


//Local imports
#include "CppUnitTest.h"
#include "../sha256gpu/GPUController.cpp"
#include "../sha256gpu/hashSingle.cpp"
#include "../sha256gpu/hashMultiple.cpp"
#include "../sha256gpu/hashSingleSalted.cpp"
#include "../sha256gpu/crackSingle.cpp"
#include "../sha256gpu/crackSingleSalted.cpp"
#include "../sha256gpu/platformDetails.cpp"
#include "../sha256gpu/LinkedList.hpp"

#pragma comment(lib, "OpenCL.lib")

#define KERNEL_DIR "../../../SHA256GPU/"


using Assert = Microsoft::VisualStudio::CppUnitTestFramework::Assert;


namespace tests
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
		}
	};



	TEST_CLASS(LinkedListTest)
	{
	public:

		TEST_METHOD(Creating)
		{
			LinkedList<std::string>* ll = new LinkedList<std::string>();
			Assert::IsNotNull(ll);

			delete ll;
		}

		TEST_METHOD(Filling)
		{
			LinkedList<std::string>* ll = new LinkedList<std::string>();

			ll->add("a");
			ll->add("b");
			ll->add("c");
			Assert::IsTrue(ll->first("a"));
			Assert::AreEqual(3, ll->count());

			delete ll;
		}

		TEST_METHOD(Popping)
		{
			LinkedList<std::string>* ll = new LinkedList<std::string>();

			ll->add("x");
			ll->add("y");
			ll->add("z");

			Assert::AreEqual(std::string("x"), ll->get());
			ll->pop();
			Assert::AreEqual(std::string("y"), ll->get());
			ll->pop();
			Assert::AreEqual(std::string("z"), ll->get());
			ll->pop();
			Assert::AreEqual(std::string(""), ll->get());

			Assert::AreEqual(0, ll->count());

			delete ll;
		}

		TEST_METHOD(Exporting)
		{
			LinkedList<std::string>* ll = new LinkedList<std::string>();

			ll->add("a");
			ll->add("b");
			ll->add("c");

			std::vector<std::string> out;
			ll->dump(out);
			Assert::AreEqual((size_t)3, out.size());

			Assert::AreEqual(std::string("a"), out[0]);
			Assert::AreEqual(std::string("b"), out[1]);
			Assert::AreEqual(std::string("c"), out[2]);

			delete ll;
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
		}
	};
}