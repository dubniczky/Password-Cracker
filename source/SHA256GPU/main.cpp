//Debug command line arguments
/*
platform
hash single banana
hash single banana xyzw
hash multiple ../../passwords/passwords-100k.txt result.txt

--banana hash
crack single ../../passwords/passwords-100.txt b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e
crack single ../../passwords/passwords-100k.txt b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e
crack single ../../passwords/passwords-4m.txt b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e

--gulyasleves hash
crack single ../../passwords/passwords-100.txt 15062cd57ae7b7eaf0aa3262fb44428a13bfc4686fcd84ba4a7a092bf7556e42
crack single ../../passwords/passwords-100k.txt 15062cd57ae7b7eaf0aa3262fb44428a13bfc4686fcd84ba4a7a092bf7556e42
crack single ../../passwords/passwords-4m.txt 15062cd57ae7b7eaf0aa3262fb44428a13bfc4686fcd84ba4a7a092bf7556e42

--4m last match
crack single ../../passwords/passwords-4m.txt 59557cf1890bf0b7458c1e66119ab01c3a796fd09df296ef7e70745d29934777
--100k last match
crack single ../../passwords/passwords-100k.txt c79c99dded78b97103916e94e5bc052d0b881ad2da896674b177bda1b1830e35

--banana + fc9h6fsd salt
crack single ../../passwords/passwords-100.txt fc9h6fsd1503052662d44b5f4aecd520103e84da17d1bf45579ade29abddc483efed07b1
--encloses + Hf45DD salt
crack single ../../passwords/passwords-100k.txt Hf45DD9c9e82db146a9bfe73b43aebfa89cd889a4fccc6fe916a66dcd497ecc4c182a2
*/

#include "GPUController.hpp"

int main(int argc, char* argv[])
{
	GPUController* gpuc;

	//No arguments
	if (argc < 2)
	{
		printf("platform                                    : list platforms\n");
		printf("hash single <password>                      : hash a single password\n");
		printf("hash single <password> <salt>               : hash a single password with salt\n");
		printf("hash multiple <dictionary.txt> <output.txt> : hash multiple passwords\n");
		printf("crack single <dictionary.txt> <hash>        : hash multiple passwords\n");
		return 0;
	}

	//Platform
	if (strcmp(argv[1], "platform") == 0)
	{
		gpuc = new GPUController();
		gpuc->platform();
	}
	//Hash
	else if (strcmp(argv[1], "hash") == 0)
	{
		if (argc < 3)
		{
			printf("single or multiple\n");
			return 0;
		}

		if (strcmp(argv[2], "single") == 0)
		{
			if (argc < 4)
			{
				printf("<password>\n");
				return 0;
			}

			if (argc < 5)
			{
				string key(argv[3]);
				gpuc = new GPUController(0);
				gpuc->singleHash(key);
			}
			else
			{
				string key(argv[3]);
				string salt(argv[4]);
				gpuc = new GPUController();
				gpuc->singleHashSalted(key, salt);
			}
		}
		else if (strcmp(argv[2], "multiple") == 0)
		{
			if (argc < 5)
			{
				printf("<input.txt> <output.txt>\n");
				return 0;
			}
			else
			{
				string infile(argv[3]);
				string outfile(argv[4]);

				gpuc = new GPUController();
				gpuc->multiHash(infile, outfile);
			}
		}
		else
		{
			printf("single or multiple\n");
			return 0;
		}
	}
	//Crack
	else if (strcmp(argv[1], "crack") == 0)
	{
		if (argc < 3)
		{
			printf("single or multiple\n");
			return 0;
		}

		if (strcmp(argv[2], "single") == 0)
		{
			if (argc < 5)
			{
				printf("<dictionary.txt> <hash>\n");
				return 0;
			}

			string sourceFile(argv[3]);
			string hash(argv[4]);
			gpuc = new GPUController();
			gpuc->crackSingle(sourceFile, hash);
		}
	}

	return 0;
}