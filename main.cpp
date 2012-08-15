/* 
 * File:   main.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 11, 2012
 */

#include <iostream>
#include <fstream>
#include <string>
#include "huffman.h"

using std::cout;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;

// returns the extension with dot of a file path,
// or the empty string if there's no dot or the extension is longer than 3 chars
inline string getExtension(const string& path)
{
    string::size_type index = path.rfind(".");
    
    if(index != string::npos && index+3 < path.length())
    {
        return path.substr(index, 4);
    }
    else
    {
        return "";
    }
}

int main(int argc, char** argv)
{
    // if incorrect num of args is given, yell at user
    if(argc != 2)
    {
        cerr << "huffman must be passed exactly one argument: "
                "the path to a file to encode or decode.\n";
        return 1;
    }
    else // exactly one arg was passed
    {   
        string path (argv[1]);
        char errorCode = 0;
        
        if(getExtension(path) == ".huf")
        {
            // decode
            errorCode = huffman::decode(argv[1], "not yet used");
        }
        else
        {
            // encode
            errorCode = huffman::encode(argv[1], path.append(".huf").c_str());
        }
        
        switch(errorCode)
        {
            case 0: // no error
                break;
            case 1: // error in input file path
                cerr << "Failed to open input file.\n";
                break;
            case 2:
                cerr << "Failed to open output file.\n";
                break;
            default:
                cerr << "Unknown error";
                break;
        }
        
        return errorCode;
    }
    
    return 0;
}

