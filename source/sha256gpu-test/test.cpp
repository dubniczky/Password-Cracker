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
#include "../sha256gpu/ArgList.hpp"

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

            //Attaching default device
			Assert::IsTrue(gc->attachDevice({}));

			//Valid kernel properties
			Assert::IsTrue( KernelProperties(0, 0, 5000, 22).valid());
			Assert::IsTrue( KernelProperties(1, 3, 10, 3).valid());
			Assert::IsFalse(KernelProperties(-1, 3, 10, 3).valid());
			Assert::IsFalse(KernelProperties(0, 0, 5, 0).valid());
			Assert::IsFalse(KernelProperties(0, 0, 0, 5).valid());

			//Attach with kernel properties
			Assert::IsTrue(gc->attachDevice(KernelProperties(0, 0, 5000, 22)));

            

			delete gc;
		}

		TEST_METHOD(HashSingle)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice({});

			//Hash
			Assert::AreEqual(std::string("b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e"), gc->hashSingle("banana"));
			Assert::AreEqual(std::string("c79c99dded78b97103916e94e5bc052d0b881ad2da896674b177bda1b1830e35"), gc->hashSingle("encloses"));
			Assert::AreEqual(std::string("9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2"), gc->hashSingle("enclosesHf45DD"));
			Assert::AreEqual(std::string("59557cf1890bf0b7458c1e66119ab01c3a796fd09df296ef7e70745d29934777"), gc->hashSingle("ex-wethouder"));

			//Length mods (4)
			Assert::AreEqual(std::string("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb"), gc->hashSingle("a")); //1
			Assert::AreEqual(std::string("961b6dd3ede3cb8ecbaacbd68de040cd78eb2ed5889130cceb4c49268ea4d506"), gc->hashSingle("aa")); //2
			Assert::AreEqual(std::string("9834876dcfb05cb167a5c24953eba58c4ac89b1adf57f28f2f9d09af107ee8f0"), gc->hashSingle("aaa")); //3
			Assert::AreEqual(std::string("61be55a8e2f6b4e172338bddf184d6dbee29c98853e0a0485ecee7f27b9af0b4"), gc->hashSingle("aaaa")); //0
			Assert::AreEqual(std::string("ed968e840d10d2d313a870bc131a4e2c311d7ad09bdf32b3418147221f51a6e2"), gc->hashSingle("aaaaa")); //1

			delete gc;
		}

		TEST_METHOD(HashSingleSalted)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice({});

			//Hash
			Assert::AreEqual(std::string("9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2"), gc->hashSingleSalted("encloses", "Hf45DD"));
			Assert::AreEqual(std::string("1503052662d44b5f4aecd520103e84da17d1bf45579ade29abddc483efed07b1"), gc->hashSingleSalted("banana", "fc9h6fsd"));
			Assert::AreEqual(std::string("c26a927094eeef581fddd140e3758691a8601182f99bb0d1e2d3ecda360bcca9"), gc->hashSingleSalted("csipke", "bokor"));
			Assert::AreEqual(std::string("7ff12cef54d78f7f82188b7c437ca88a3bf66d7850ebe6b429023a78c26d5783"), gc->hashSingleSalted("bestpassword", "ornot"));
			Assert::AreEqual(std::string("e5f23788c7dd827f301b9c0827ff8111552e361d65de8c4035d51df66433bec4"), gc->hashSingleSalted("secureaf", "GlOuin5"));
			Assert::AreEqual(std::string("8d2b8da91ff28558d835c17ef91c07000bc58896ef7b98710c18dea92afb836f"), gc->hashSingleSalted("ex-wethouder", "salty"));

			delete gc;
		}

		TEST_METHOD(CrackSingle)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice({});

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

			//Invalid character
			Assert::AreEqual(std::string(""),
				gc->crackSingle(pass100k, "k493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e"));

			//Short hash
			Assert::AreEqual(std::string(""), 
				gc->crackSingle(pass100k, "c79c99dded78b97103916e94e5bc052d0b881ad2da896674bda1b1830e35"));

			delete gc;
		}


		TEST_METHOD(CrackSingleSalted)
		{
			//Load
			GPUController* gc = new GPUController();
			gc->attachDevice({});

			//Crack
			std::string pass100k = "../../../../passwords/passwords-100k.txt";
			std::string pass4m = "../../../../passwords/passwords-4m.txt";

			Assert::AreEqual(std::string("banana"), //salt: asdf //Found the banana!
				gc->crackSingleSalted(pass100k, "asdf7d9d10127d2c1a49684e2d2f258e1dc1ccf05649b5690dd55c311afe9ad16441"));
			Assert::AreEqual(std::string("encloses"), //salt: Hf45DD
				gc->crackSingleSalted(pass100k, "Hf45DD9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2"));
			Assert::AreEqual(std::string("ex-wethouder"), //salt: salty
				gc->crackSingleSalted(pass4m, "salty8d2b8da91ff28558d835c17ef91c07000bc58896ef7b98710c18dea92afb836f"));

			//Missing from passwords list (vizslakutya + abcd) hash
			Assert::AreEqual(std::string(""),
				gc->crackSingleSalted(pass100k, "abcd1b72955fcf8258e459e16961bde674e7a364b4d85bce939eea774dd7250de06a"));

			delete gc;
		}
	};


	TEST_CLASS(LinkedListTest)
	{
	public:

		TEST_METHOD(Creating)
		{
			ArgList* ll = new ArgList();
			Assert::IsNotNull(ll);

			delete ll;
		}

		TEST_METHOD(Filling)
		{
			ArgList* ll = new ArgList();

			ll->add("a");
			ll->add("b");
			ll->add("c");
			Assert::IsTrue(ll->first("a"));
			Assert::AreEqual(3, ll->count());

			delete ll;
		}

		TEST_METHOD(Popping)
		{
			ArgList* ll = new ArgList();

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
			ArgList* ll = new ArgList();

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
	};
}