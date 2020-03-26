//Nagy Richard Antal
//V7BFDU

//Preprocessor requires: _CRT_SECURE_NO_WARNINGS flag to enable spritf
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include "sha256.h"

using std::string;
using std::ifstream;
using std::vector;
using namespace std::chrono;

const unsigned int hashSize = 64;

string readFile(string name)
{
    ifstream infile(name);
    return { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
}

int main(int argc, char* argv[])
{
    //Guards
    if (argc < 3)
    {
        printf("2 parameters required.\ncrack.exe <target-file> <table-file>\n");
        return 1;
    }

    //Read target
    string targetFile = argv[1];    
    string target = readFile(targetFile);
    printf("Target: '%s'\n", target.c_str());
    if (target.length() < hashSize)
    {
        printf("Hash must be at lease 64 characters long\n");
        return 2;
    }
    int saltSize = target.length() - hashSize;
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
    
    return 0;
}