//Nagy Richard Antal
//V7BFDU

//Preprocessor requires: _CRT_SECURE_NO_WARNINGS flag to enable spritf

#include <stdio.h>
#include <fstream>
#include <sstream>
#include "sha256.h"

using std::string;

int main(int argc, char* argv[])
{
    std::ifstream input("passwords-20.txt");

    string pass;
    for (int i = 0; std::getline(input, pass); i++)
    {
        printf("%s\n", sha256(pass).c_str());
    }
    printf("ok");

    //string input = "grape";
    //string output1 = sha256(input);

    //cout << "sha256('" << input << "'):" << output1 << endl;
    //printf("%s %s %s", );
    return 0;
}