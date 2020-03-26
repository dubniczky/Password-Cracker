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

string readFile(string name)
{
    ifstream infile(name);
    return { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("2 parameters required.\ncrack.exe <target-file> <table-file>");
        return 1;
    }
    string targetName = argv[1];
    string tableName = argv[2];
    string target = readFile(targetName);
    string current;
    bool match = false;

    //Reading hash table
    vector<string> hashTable;
    for (ifstream input(tableName); getline(input, current);)
    {
        hashTable.push_back(current);
    }

    //Matching
    printf("Matching...");
    auto startTime = high_resolution_clock::now();
    int index;
    for (index = 0; index < hashTable.size(); index++)
    {
        string hash = sha256(hashTable[index]);
        if (target == hash) break;
    }
    
    //Measure length
    auto stopTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stopTime - startTime);
    long long microseconds = duration.count();

    //Check result
    if (index < hashTable.size())
    {
        printf("\nMatch found: %s\n", hashTable[index].c_str());
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