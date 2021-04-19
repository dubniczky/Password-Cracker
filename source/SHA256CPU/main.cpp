/* Tests

Hashing:
../../passwords/passwords-100k.txt hashes.txt

Cracking:
../../passwords/passwords-4m.txt 8d2b8da91ff28558d835c17ef91c07000bc58896ef7b98710c18dea92afb836f salty


*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <string>
#include <map>

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
int crackKeySalted(std::string file, std::string hash, std::string salt)
{
    std::cout << "Cracking..." << std::endl;

    auto startTime = high_resolution_clock::now();
    std::ifstream infile(file);
    sha256* context = new sha256();
    context->storedHash = hash;

    std::string key;
    bool match = false;
    for (; getline(infile, key) ;)
    {
        if (context->verify(key.length() + salt.length(), key + salt))
        {
            match = true;
            break;
        }        
    }

    //Measure length
    auto stopTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stopTime - startTime);
    long long microseconds = duration.count();

    std::cout << "Cracking completed." << std::endl;
    std::cout << "Time: " << microseconds << " microseconds" << std::endl;

    if (match)
    {
        std::cout << "Match found:" << std::endl << key << std::endl;
    }
    else
    {
        std::cout << "Match not found." << std::endl;
    }

    delete context;
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
        return crackKeySalted(argv[1], argv[2], argv[3]);
    }
    
    std::cout << "Invalid parameters." << std::endl <<
                 "<infile> <outfile>" << std::endl <<
                 "<infile> <hash> <salt>" << std::endl;
}
