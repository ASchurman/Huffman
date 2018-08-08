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

BitFileOut::BitFileOut(const std::string& filePath)
{
    open(filePath);
}

BitFileOut::~BitFileOut()
{
    if (isOpen())
    {
        close();
    }
}

bool BitFileOut::open(const std::string& filePath)
{
    outfile.open(filePath, ios::in | ios::out | ios::binary | ios::trunc);
    return outfile.good();
}

void BitFileOut::close()
{
    // We can't close if we're already closed.
    if (!isOpen())
    {
        return;
    }
    
    // First, determine the number of unused bits at the end of the last byte.
    unsigned char numUnused = 0;

    // If the next bit to be written is the most-significant-bit, then there's
    // currently no bits in the buffer and thus no unused bits.
    if (nextBit != 0x80)
    {
        for (numUnused = 0; nextBit != 0; nextBit >>= 1)
        {
            numUnused++;
        }
    }

    // Next, flush the buffer to file if there's anything in the buffer.
    if (numUnused > 0)
    {
        outfile.put((char)buffer);
    }

    // Finally, write my unused bits to the beginning of the file and close it.
    outfile.seekp(0);
    unsigned char firstByte = outfile.peek();
    firstByte |= numUnused << 5; // Shift 5 moves numUnused to 3 most-sig-bits
    outfile.put(firstByte);

    outfile.close();
}

void BitFileOut::writeBit(unsigned char bit)
{
    if (bit)
    {
        buffer |= nextBit;
    }

    nextBit >>= 1;

    // If nextBit is 0, then we've right-shifted the 1 off of it, meaning the
    // buffer is full and needs to be flushed.
    if (nextBit == 0)
    {
        outfile.put(buffer);
        nextBit = 0x80;
        buffer = 0x00;
    }
}

void BitFileOut::writeByte(unsigned char bits)
{
    for (unsigned char mask = 0x80; mask != 0; mask >>= 1)
    {
        writeBit(bits & mask);
    }
}
