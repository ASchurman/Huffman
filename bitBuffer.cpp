/* 
 * File:   bitBuffer.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 * 
 * Created on August 20, 2012
 */

#include "bitBuffer.h"

#include <fstream>
#include <vector>

using std::ios;
using std::vector;
using std::ofstream;

bitBuffer::bitBuffer()
{
    outfile = new ofstream();
    
    buffer = new vector<unsigned char>();
    buffer->push_back(0x00);
    
    // reserve 3 bits to indicate the number of unused bits at the end
    // of the last byte
    mask = 0x10; // binary 0001 0000
}

bitBuffer::bitBuffer(const bitBuffer& orig)
{
    outfile = orig.outfile;
    buffer = orig.buffer;
    mask = orig.mask;
}

bitBuffer::~bitBuffer()
{
    if(outfile->is_open())
    {
        outfile->close();
    }
    delete outfile;
    
    delete buffer;
}

bool bitBuffer::open(const char* outputFilePath)
{
    outfile->open(outputFilePath, ios::out | ios::trunc | ios::binary);
    
    return outfile->good();
}

void bitBuffer::close()
{
    // check for openness; we can only close files that are open
    if(!outfile->is_open())
    {
        return;
    }
    
    // determine the number of unused bits at the end of the last byte
    unsigned char numUnused;
    for(numUnused = 0; mask != 0; mask >>= 1)
    {
        numUnused++;
    }
    
    // write the num of unused bits to the first 3 bits of the buffer
    // that were reserved upon construction
    numUnused <<= 5; // shift to 3 most significant bits
    buffer->at(0) |= numUnused;
    
    // write the buffer vector into outfile
    outfile->write(reinterpret_cast<char*>(&(buffer->front())), buffer->size());
    
    outfile->close();
}

// adds a new byte to the buffer if we need one
inline void bitBuffer::checkBuffer()
{
    // zero mask means that we've right-shifted the 1 off of it;
    // it's time to push a new byte on
    if(mask == 0)
    {
        buffer->push_back(0x00);
        mask = 0x80; // binary 1000 0000
    }
}

void bitBuffer::write(const char* bits, char numBits)
{
    for(int i = 0; i < numBits; i++)
    {
        checkBuffer(); // add a new byte if it's needed
        
        // we only need to modify the current byte if we're
        // writing a 1; the bytes are initialized to 0 in checkBuffer()
        if(bits[i] == 1)
        {
            buffer->at(buffer->size() - 1) |= mask;
        }
        
        // update the mask
        mask >>= 1;
    }
}

void bitBuffer::write(unsigned char bits)
{
    for(unsigned char i = 0x80; i != 0; i >>= 1)
    {
        checkBuffer(); // adds a new byte if it's needed
        
        if((bits & i) != 0)
        {
            buffer->at(buffer->size() - 1) |= mask;
        }
        
        // update the mask
        mask >>= 1;
    }
}