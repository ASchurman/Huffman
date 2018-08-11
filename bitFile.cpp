/* 
 * File:   bitFile.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 */

#include "bitFile.h"

#include <fstream>
#include <vector>

using std::ios;
using std::vector;

/*******************************************************************************
******************************** BitFileOut ************************************
*******************************************************************************/

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
    if (isOpen())
    {
        return false;
    }
    else
    {
        outfile.open(filePath, ios::in | ios::out | ios::binary | ios::trunc);
        return outfile.good();
    }
}

bool BitFileOut::close()
{
    // We can't close if we're already closed.
    if (!isOpen())
    {
        return false;
    }
    
    bool success = true;

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
        success = outfile.good();
    }

    // Finally, write my unused bits to the beginning of the file and close it.
    if (success)
    {
        outfile.seekp(0);
        unsigned char firstByte = outfile.peek();
        firstByte |= numUnused << 5; // Shift moves numUnused to first 3 bits
        outfile.put(firstByte);
        success = outfile.good();
    }

    outfile.close();
    return success;
}

bool BitFileOut::writeBit(unsigned char bit)
{
    bool success = true;

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
        success = outfile.good();

        nextBit = 0x80;
        buffer = 0x00;
    }

    return success;
}

bool BitFileOut::writeBits(const vector<bool>& bits)
{
    bool success = true;

    for (auto it = bits.cbegin(); it != bits.cend() && success; it++)
    {
        if (*it)
        {
            success = writeBit(0x01);
        }
        else
        {
            success = writeBit(0x00);
        }
    }

    return success;
}

bool BitFileOut::writeByte(unsigned char bits)
{
    bool success = true;

    for (unsigned char mask = 0x80; mask != 0 && success; mask >>= 1)
    {
        success = writeBit(bits & mask);
    }

    return success;
}

/*******************************************************************************
******************************** BitFileIn *************************************
*******************************************************************************/

BitFileIn::BitFileIn(const std::string& filePath)
{
    open(filePath);
}

BitFileIn::~BitFileIn()
{
    if (isOpen())
    {
        close();
    }
}

bool BitFileIn::open(const std::string& filePath)
{
    bool success = false;

    if (!isOpen())
    {
        infile.open(filePath, ios::in | ios::binary);
        if (infile.good())
        {
            buffer = infile.get();
        }
        success = infile.good();

        if (success)
        {
            // The first 3 bits of the file indicate the number of unused,
            // excess bits at the end of the last byte of the file.
            numRemainderBits = buffer >> 5;

            // Begin reading from the buffer on the 4th bit.
            nextBit = 0x10;
        }
    }
    return success;
}

void BitFileIn::close()
{
    if (isOpen())
    {
        infile.close();
    }
}

std::pair<bool, unsigned char> BitFileIn::readBit()
{
    bool readSuccess = false;
    unsigned char bitOut = 0x00;

    if (isOpen() && !eof())
    {
        readSuccess = true;

        // If our 1 in nextBit has been shifted off the end, that means that
        // we've read everything in the buffer and need to read a new byte
        // from file.
        if (nextBit == 0x00)
        {
            readSuccess = readToBuffer();
        }

        if (readSuccess)
        {
            if ((buffer & nextBit) == 0)
            {
                bitOut = 0x00;
            }
            else
            {
                bitOut = 0x01;
            }
            nextBit >>= 1;
        }
    }

    return std::make_pair(readSuccess, bitOut);
}

bool BitFileIn::readToBuffer()
{
    bool readSuccess = false;

    if (isOpen())
    {
        buffer = infile.get();
        readSuccess = infile.good();

        if (readSuccess)
        {
            nextBit = 0x80;
        }
    }

    return readSuccess;
}

vector<bool> BitFileIn::readBits(int numBitsToRead)
{
    std::vector<bool> bitsOut;

    // Read bits until we've read numBitsToRead or until we fail to read
    // another bit
    for (int i = 0; i < numBitsToRead; i++)
    {
        auto [readSuccess, bitRead] = readBit();
        if (readSuccess)
        {
            bitsOut.push_back((bool)bitRead);
        }
        else
        {
            break;
        }
    }

    return bitsOut;
}

bool BitFileIn::eof()
{
    // (1) We can only be EOF if we're open.
    // (2) We can only be EOF if our infile is EOF.
    // (3) If the next bit to be read from the buffer is within numRemainderBits
    //     from the end of the buffer, then the next bit is an unused, remainder
    //     bit that shouldn't be read, and we're EOF.
    return isOpen() && infile.eof() && nextBit >> numRemainderBits == 0;
}
