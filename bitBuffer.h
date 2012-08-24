/* 
 * File:   bitBuffer.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 20, 2012
 */

#ifndef BITBUFFER_H
#define	BITBUFFER_H

#include <fstream>
#include <vector>

class bitBuffer {
public:
    bitBuffer();
    bitBuffer(const bitBuffer& orig);
    virtual ~bitBuffer();
    
    // opens the given file path, returning true if successful
    bool open(const char* outputFilePath);
    
    // finalizes the output file and prepares the bitBuffer for deletion
    void close();
    
    // writes the given bits to the file, bits[0] first
    // and going up to bits[numBits-1]
    void write(const char* bits, char numBits);
    
    // writes a single byte to the file
    void write(unsigned char bits);
    
private:
    std::ofstream* outfile;
    std::vector<unsigned char>* buffer;
    unsigned char mask;
    
    inline void checkBuffer();
};

#endif	/* BITBUFFER_H */

