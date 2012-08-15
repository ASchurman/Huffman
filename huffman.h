/* 
 * File:   huffman.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 13, 2012
 * 
 * Contains functions to encode and decode files using Huffman coding.
 */

#include <fstream>

#ifndef HUFFMAN_H
#define	HUFFMAN_H

namespace huffman
{
    // encodes given input file path into given output file path.
    // returns 0 if successful, 1 if inpath is invalid, 2 if outpath is invalid
    char encode(const char* inpath, const char* outpath);
    
    // decodes given input file path into given output file path.
    // returns 0 if successful, 1 if inpath is invalid, 2 if outpath is invalid
    char decode(const char* inpath, const char* outpath);
}


#endif	/* HUFFMAN_H */

