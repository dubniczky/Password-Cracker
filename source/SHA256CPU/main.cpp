/* Tests

Hashing:
../../passwords/passwords-100k.txt hashes.txt



*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <string>

#include "sha256.hpp"


using std::vector;
using namespace std::chrono;


//Sha256
int hashKeys(std::string inFile, std::string outFile)
{
    std::cout << "Hashing..." << std::endl;

    auto startTime = high_resolution_clock::now();
    std::ifstream infile(inFile);
    std::ofstream outfile(outFile);
    sha256* context = new sha256();


    for (std::string key; getline(infile, key); )
    {
        std::string hash = context->hash(key.length(), key);
        //std::cout << key << " ";
        outfile << hash << std::endl;
    }

    //Measure length
    auto stopTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stopTime - startTime);
    long long microseconds = duration.count();

    std::cout << "Hashing completed." << std::endl;
    std::cout << "Time: " << microseconds << " microseconds" << std::endl;

    delete context;
    return 0;
}
int crackKey(std::string file, std::string hash, unsigned int batchSize = 1000)
{
    /*
    //Read target
    string targetFile = argv[1];
    string target = readFile(targetFile);
    printf("Target: '%s'\n", target.c_str());
    if (target.length() < 64)
    {
        printf("Hash must be at lease 64 characters long\n");
        return 2;
    }
    int saltSize = target.length() - 64;
    string salt = target.substr(0, saltSize);
    string hash = target.substr(saltSize, hashSize);

    printf("Salt: '%s'\n", salt.c_str());
    printf("Hash: '%s'\n", hash.c_str());

    //Reading password table
    printf("Reading password table...\n");
    vector<string> passTable;
    string tableFile = argv[2];
    string current;
    for (ifstream input(tableFile); getline(input, current);)
    {
        passTable.push_back(current);
    }

    //Matching
    printf("Matching...");
    auto startTime = high_resolution_clock::now();
    bool match = false;
    int index;
    for (index = 0; index < (int)passTable.size(); index++)
    {
        string chash = sha256(passTable[index] + salt);
        if (hash == chash) break;
    }

    //Measure length
    auto stopTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stopTime - startTime);
    long long microseconds = duration.count();

    //Check result
    if (index < (int)passTable.size())
    {
        printf("\nMatch found: %s\n", passTable[index].c_str());
        printf("Checked lines: %i\n", index + 1);
        printf("Search time: %lld microseconds\n", microseconds);
    }
    else
    {
        printf("\nMatch not found\n");
        printf("Checked lines: %i\n", index);
        printf("Search time: %lld microseconds\n", microseconds);
    }
    */

    return 0;
}


//Main
int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        return hashKeys(argv[1], argv[2]);
    }
    if (argc == 4)
    {
        return crackKey(argv[1], argv[2], std::stoi(argv[3]));
    }
    
    std::cout << "Invalid parameters." << std::endl <<
                 "<infile> <outfile>" << std::endl <<
                 "<infile> <hash> <batch-size>" << std::endl;
}
