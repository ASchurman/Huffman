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
******************************** BitVector *************************************
*******************************************************************************/

void BitVector::pushBack(bool bit)
{
    bitstore.push_back(bit);
}

void BitVector::pushBack(unsigned char byte)
{
    for (unsigned char mask = 0x80; mask != 0; mask >>= 1)
    {
        pushBack((byte & mask) > 0);
    }
}

void BitVector::pushFront(bool bit)
{
    bitstore.insert(bitstore.cbegin(), bit);
}

void BitVector::pushFront(unsigned char byte)
{
    for (unsigned char mask = 0x01; mask != 0; mask <<= 1)
    {
        pushFront((byte & mask) > 0);
    }
}

bool BitVector::popfrontByte(unsigned char& byteOut)
{
    bool success = canPopByte();
    byteOut = 0x00;

    if (success)
    {
        for (unsigned char mask = 0x80; mask > 0; mask >>= 1)
        {
            byteOut |= mask & (bitstore.front() ? 0xFF : 0x00);
            bitstore.erase(bitstore.begin());
        }
    }
    return success;
}

bool BitVector::popfrontBit(unsigned char& bitOut)
{
    bool success = canPopBit();
    bitOut = 0x00;

    if (success)
    {
        bitOut = bitstore.front() ? 0x01 : 0x00;
        bitstore.erase(bitstore.begin());
    }
    return success;
}

void BitVector::clear()
{
    bitstore.clear();
}

/*******************************************************************************
******************************** BitFileOut ************************************
*******************************************************************************/

BitFileOut::BitFileOut()
{
    initBuffer();
}

BitFileOut::BitFileOut(const std::string& filePath)
{
    initBuffer();
    open(filePath);
}

void BitFileOut::initBuffer()
{
    // Pad the front of the first byte by 3 bits. When we close the file, we
    // will write in these bits the number of excess, unused bits at the end
    // of the final byte.
    buffer.pushBack(false);
    buffer.pushBack(false);
    buffer.pushBack(false);
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
    unsigned char numUnused = (8 - buffer.remainderBits()) % 8;

    // Next, flush the buffer to file. In addition to flushing the whole bytes
    // from the buffer, we also need to flush the remainder bits.
    flushBuffer();
    if (buffer.canPopBit())
    {
        unsigned char finalByte = 0x00;
        unsigned char mask = 0x80;
        while (buffer.canPopBit())
        {
            unsigned char bitOut;
            buffer.popfrontBit(bitOut);
            if (bitOut)
            {
                finalByte |= mask;
            }
            mask >>= 1;
        }

        outfile.put(finalByte);
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

    buffer.clear();
    outfile.close();
    return success;
}

bool BitFileOut::flushBuffer()
{
    if (!isOpen())
    {
        return false;
    }

    bool success = true;
    while (buffer.canPopByte() && success)
    {
        unsigned char byteOut;
        buffer.popfrontByte(byteOut);
        outfile.put(byteOut);
        success = outfile.good();
    }
    return success;
}

bool BitFileOut::writeBit(unsigned char bit)
{
    if (!isOpen())
    {
        return false;
    }

    bool success = true;

    buffer.pushBack((bool)bit);

    if (buffer.numBits() >= bufferCapacity)
    {
        success = flushBuffer();
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
            // Get the first byte from infile in order to extract the first 3
            // bits, indicating the number of excess bits at the end of the
            // last byte. Then from there read into the buffer.
            unsigned char firstByte = infile.get();
            if (infile.good())
            {
                numRemainderBits = (firstByte & 0xE0) >> 5;
                for (unsigned char mask = 0x10; mask != 0; mask >>= 1)
                {
                    buffer.pushBack((bool)(firstByte & mask));
                }

                success = readToBuffer();
            }
        }
    }
    return success;
}

bool BitFileIn::readToBuffer()
{
    bool readSuccess = false;

    if (isOpen())
    {
        vector<unsigned char> readBytes(bufferCapacityBytes);
        infile.read((char*)readBytes.data(), bufferCapacityBytes);
        int numRead = infile.gcount();

        if (numRead > 0)
        {
            // Put the read data into the bit buffer
            readSuccess = true;
            for (int i = 0; i < numRead - 1; i++)
            {
                buffer.pushBack(readBytes[i]);
            }

            // The final byte read may be the final byte in the file. If it is,
            // we need to handle it differently.
            if (infile.peek() == EOF)
            {
                // Since we just read to the end of the file, don't put the
                // excess bits in the last byte into the buffer.
                unsigned char mask = 0x80;
                unsigned char finalByte = readBytes[numRead - 1];
                for (int i = 0; i < 8 - numRemainderBits; i++)
                {
                    buffer.pushBack((bool)(finalByte & mask));
                    mask >>= 1;
                }
            }
            else
            {
                // The final byte read isn't the last byte in the file, so
                // put the entire byte into the bit buffer.
                buffer.pushBack(readBytes[numRead - 1]);
            }
        }
    }

    return readSuccess;
}

void BitFileIn::close()
{
    if (isOpen())
    {
        infile.close();
        buffer.clear();
    }
}

bool BitFileIn::readBit(unsigned char& bitOut)
{
    bool readSuccess = canRead();
    bitOut = 0x00;

    if (readSuccess)
    {
        if (!buffer.canPopBit())
        {
            readSuccess = readToBuffer();
        }

        if (readSuccess)
        {
            readSuccess = buffer.popfrontBit(bitOut);
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
        unsigned char bitRead;
        if (readBit(bitRead))
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

bool BitFileIn::readByte(unsigned char& byteOut)
{
    byteOut = 0x00;
    bool success = isOpen();

    if (success)
    {
        // If we have fewer than 8 bits in the buffer, try refilling the buffer
        if (!buffer.canPopByte())
        {
            readToBuffer();
        }

        // If we have now have enough bits in the buffer, read them out. But if
        // we still don't have enough after reading into buffer, fail.
        if (buffer.canPopByte())
        {
            success = buffer.popfrontByte(byteOut);
        }
        else
        {
            success = false;
        }
    }
    return success;
}

bool BitFileIn::canRead()
{
   // (1) We can only read if we're open.
   // (2) We need to have bits to read either in the buffer or the file.
   return isOpen()
          && (buffer.canPopBit() || infile.peek() != EOF);
}
