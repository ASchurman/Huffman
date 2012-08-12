/* 
 * File:   main.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 11, 2012
 */

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// returns the extension with dot of a file path,
// or the empty string if there's no dot or the extension is longer than 3 chars
inline string getExtension(string path)
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

inline void fileDNE()
{
    cerr << "Failed to open given file path.\n";
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
        ifstream input;
        
        if(getExtension(path) == ".huf")
        {
            // decode
            try
            {
                input.open(argv[1], ios::in | ios::binary);
            }
            catch(ifstream::failure e)
            {
                fileDNE();
                return 1;
            }
            
            // TODO: invoke decode function
        }
        else
        {
            // encode
            try
            {
                input.open(argv[1], ios::in);
            }
            catch(ifstream::failure e)
            {
                fileDNE();
                return 1;
            }
            
            // TODO: invoke encode function
        }
        
        input.close();
    }
    
    return 0;
}

