/*
File: bitfileTest.cpp
Author: Alexander Schurman, alexander.schurman@gmail.com

Provides tests for the classes defined in bitFile.h
*/

#include "catch.hpp"
#include "../bitFile.h"
#include <fstream>
#include <string>
#include <cstdio>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <sstream>

std::string hexVar(const std::string& varName, unsigned int var)
{
    std::ostringstream s;
    s << varName << ": 0x" << std::hex << var << "\n";
    return s.str();
}

TEST_CASE("operating on BitVector", "[bitfile][BitVector]")
{
    bool bit = true;
    unsigned char byte = 0xA1;
    unsigned char popped = 0x00;

    // Operating on empty BitVector
    BitVector v;
    REQUIRE(v.empty());
    REQUIRE(!v.canPopBit());
    REQUIRE(!v.canPopByte());
    REQUIRE(!v.popfrontBit(popped));
    REQUIRE(popped == 0x00);
    REQUIRE(!v.popfrontByte(popped));
    REQUIRE(popped == 0x00);

    // Push a bit
    v.pushBack(bit);
    REQUIRE(v.numBits() == 1);
    REQUIRE(!v.empty());
    REQUIRE(v.remainderBits() == 1);
    REQUIRE(v.canPopBit());
    REQUIRE(!v.canPopByte());

    // Push a byte
    v.pushBack(byte);
    REQUIRE(v.numBits() == 9);
    REQUIRE(!v.empty());
    REQUIRE(v.remainderBits() == 1);
    REQUIRE(v.canPopBit());
    REQUIRE(v.canPopByte());

    // Pop a bit
    REQUIRE(v.popfrontBit(popped));
    INFO(hexVar("popped", popped));
    REQUIRE(popped == (bit ? 0x01 : 0x00));
    REQUIRE(v.numBits() == 8);
    REQUIRE(!v.empty());
    REQUIRE(v.remainderBits() == 0);
    REQUIRE(v.canPopBit());
    REQUIRE(v.canPopByte());

    // Pop a byte
    REQUIRE(v.popfrontByte(popped));
    INFO(hexVar("popped", popped));
    REQUIRE(popped == byte);
    REQUIRE(v.numBits() == 0);
    REQUIRE(v.empty());
    REQUIRE(v.remainderBits() == 0);
    REQUIRE(!v.canPopBit());
    REQUIRE(!v.canPopByte());

    // Push to front
    v.pushFront(bit);
    REQUIRE(v.numBits() == 1);
    REQUIRE(!v.empty());
    v.pushFront(bit);
    REQUIRE(v.numBits() == 2);
    REQUIRE(!v.empty());
    v.pushFront(byte);
    REQUIRE(v.numBits() == 10);
    REQUIRE(!v.empty());

    // Pop to verify pushing to front
    REQUIRE(v.popfrontByte(popped));
    REQUIRE(v.numBits() == 2);
    REQUIRE(!v.empty());
    INFO(hexVar("popped", popped));
    REQUIRE(popped == byte);
    REQUIRE(v.popfrontBit(popped));
    REQUIRE(v.numBits() == 1);
    REQUIRE(!v.empty());
    INFO(hexVar("popped", popped));
    REQUIRE(popped == (bit ? 0x01: 0x00));
    REQUIRE(v.popfrontBit(popped));
    REQUIRE(v.numBits() == 0);
    REQUIRE(v.empty());
    INFO(hexVar("popped", popped));
    REQUIRE(popped == (bit ? 0x01 : 0x00));

    // Push to back then clear it
    v.pushBack(byte);
    REQUIRE(v.numBits() == 8);
    REQUIRE(!v.empty());
    v.clear();
    REQUIRE(v.numBits() == 0);
    REQUIRE(v.empty());
}

TEST_CASE("operating on closed BitFileOut", "[bitfile][BitFileOut]")
{
    BitFileOut oClosed;
    std::vector<bool> v{ true };

    REQUIRE(!oClosed.isOpen());
    REQUIRE(!oClosed.writeBit(0));
    REQUIRE(!oClosed.writeBits(v));
    REQUIRE(!oClosed.writeByte(0));

    REQUIRE(!oClosed.isOpen());
    REQUIRE(!oClosed.close());
    REQUIRE(!oClosed.isOpen());
}

TEST_CASE("operating on closed BitFileIn", "[bitfile][BitFileIn]")
{
    BitFileIn iClosed;
    REQUIRE(!iClosed.isOpen());
    REQUIRE(!iClosed.canRead());

    unsigned char bitIn;
    REQUIRE(!iClosed.readBit(bitIn));
    REQUIRE(bitIn == 0);

    std::vector<bool> bitsIn = iClosed.readBits(5);
    REQUIRE(bitsIn.empty());
}

TEST_CASE("can write to BitFileOut", "[bitfile][BitFileOut]")
{
    BitFileOut o;
    std::ifstream f;
    const std::string filename = "testBitFile1.hex";

    REQUIRE(!o.isOpen());
    o.open(filename);
    REQUIRE(o.isOpen());

    SECTION("writing individual bits")
    {
        // Write 5 bits, meaning that the entire file will be 8 bits.
        unsigned char bitsToWrite = 0x1A;
        for (unsigned char mask = 0x10; mask != 0; mask >>= 1)
        {
            unsigned char bit = (bitsToWrite & mask) > 0 ? 1 : 0;
            REQUIRE(o.writeBit(bit));
        }
        REQUIRE(o.close());
        REQUIRE(!o.isOpen());

        // Verify
        f.open(filename, std::ifstream::in | std::ifstream::binary);
        REQUIRE(f.good());

        // Ensure that the first 3 bits are 0, indicating that there are no
        // leftover bits at the end of the last (and only) byte.
        unsigned char readByte = f.get();
        INFO(hexVar("read byte", readByte).append(
             hexVar("data bits", bitsToWrite)));
        REQUIRE((readByte & 0xE0) >> 5 == 0);

        // Ensure that the correct bits were written
        REQUIRE((readByte & 0x1F) == bitsToWrite);

        // Ensure that there's only 1 byte in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

    SECTION("writing >5 individual bits")
    {
        // Write 8 individual bits, which will set the remainder bits to be >0.
        unsigned char bitsToWrite = 0x5A;
        for (unsigned char mask = 0x80; mask != 0; mask >>= 1)
        {
            unsigned char bit = (bitsToWrite & mask) > 0 ? 1 : 0;
            REQUIRE(o.writeBit(bit));
        }
        REQUIRE(o.close());
        REQUIRE(!o.isOpen());

        // Verify
        f.open(filename, std::ifstream::in | std::ifstream::binary);
        REQUIRE(f.good());

        // Ensure that the first 3 bits are 0b101, meaning that there are 5
        // excess bits at the end of the last (second) byte.
        unsigned char readByte = f.get();
        INFO(hexVar("read byte1", readByte).append(
             hexVar("data bits", bitsToWrite)));
        REQUIRE((readByte & 0xE0) >> 5 == 5);

        // Ensure that the correct data bits were written.
        REQUIRE((readByte & 0x1F) == (bitsToWrite & 0xF8) >> 3);
        readByte = f.get();
        INFO(hexVar("read byte2", readByte).append(
             hexVar("data bits", bitsToWrite)));
        REQUIRE(readByte == (bitsToWrite & 0x07) << 5);

        // Ensure that there's only 2 bytes in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

    SECTION("writing whole bytes")
    {
        unsigned char byteToWrite = 0x5A;
        REQUIRE(o.writeByte(byteToWrite));
        REQUIRE(o.close());
        REQUIRE(!o.isOpen());

        // Verify
        f.open(filename, std::ifstream::in | std::ifstream::binary);
        REQUIRE(f.good());

        // Ensure that the number of excess bits is set to 0b101
        unsigned char readByte = f.get();
        INFO(hexVar("read byte1", readByte).append(
             hexVar("data bits", byteToWrite)));
        REQUIRE((readByte & 0xE0) >> 5 == 5);

        // Ensure correct data bits were written.
        REQUIRE((readByte & 0x1F) == (byteToWrite & 0xF8) >> 3);
        readByte = f.get();
        INFO(hexVar("read byte2", readByte).append(
             hexVar("data bits", byteToWrite)));
        REQUIRE(readByte == (byteToWrite & 0x07) << 5);

        // Ensure that there's only 2 bytes in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

    SECTION("writing several bits")
    {
        std::vector<bool> boolVector {true, false, false, true, true, false};
        unsigned char bitsToWrite = 0x26;

        REQUIRE(o.writeBits(boolVector));
        REQUIRE(o.close());
        REQUIRE(!o.isOpen());

        // Verify
        f.open(filename, std::ifstream::in | std::ifstream::binary);
        REQUIRE(f.good());

        // Ensure that the number of excess bits is set to 0b111
        unsigned char readByte = f.get();
        INFO(hexVar("read byte1", readByte).append(
             hexVar("data bits", bitsToWrite)));
        REQUIRE((readByte & 0xE0) >> 5 == 7);

        // Ensure correct data bits were written.
        REQUIRE((readByte & 0x1F) == (bitsToWrite & 0x3E) >> 1);
        readByte = f.get();
        INFO(hexVar("read byte2", readByte).append(
             hexVar("data bits", bitsToWrite)));
        REQUIRE(readByte == (bitsToWrite & 0x01) << 7);

        // Ensure that there's only 2 bytes in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

    // Clean-up
    remove(filename.c_str());
}

TEST_CASE("can write lots of bits to BitFileOut", "[bitfile][BitFileOut][long]")
{
    const int numDataBytes = 5000;
    const std::string filename = "bigTestBitFile1.hex";

    BitFileOut o;
    REQUIRE(!o.isOpen());
    o.open(filename);
    REQUIRE(o.isOpen());

    // Get a large number of bytes and write it to BitFileOut
    std::vector<unsigned char> databits;
    databits.reserve(numDataBytes);
    for (int i = 0; i < numDataBytes; i++)
    {
        unsigned char randByte = rand() % 256;
        REQUIRE(o.writeByte(randByte));
        databits.push_back(randByte);
    }
    REQUIRE(o.close());
    REQUIRE(!o.isOpen());

    // Verify
    std::ifstream f;
    f.open(filename, std::ifstream::in | std::ifstream::binary);
    REQUIRE(f.good());

    // Ensure that the first 3 bits are set to 5 (0b101), indicating that
    // there are 5 excess, unused bits at the end of the last byte
    unsigned char firstByte = f.peek();
    INFO(hexVar("first byte", firstByte));
    REQUIRE((firstByte & 0xE0) >> 5 == 5);

    // Ensure correct data bits were written.
    for (int i = 0; i < numDataBytes; i++)
    {
        unsigned char readByte = f.get();
        REQUIRE(f.good());
        unsigned char dataByte = databits[i];

        // Verify 5 most-significant-bits of dataByte
        INFO(hexVar("dataByte in loop", dataByte).append(
             hexVar("readByte 1", readByte)));
        REQUIRE((readByte & 0x1F) == (dataByte >> 3));

        // Then verify 3 least-significant-bits of dataByte
        readByte = f.peek();
        REQUIRE(f.good());
        INFO(hexVar("readByte 2", readByte));
        REQUIRE((readByte & 0xE0) >> 5 == (dataByte & 0x07));
    }

    // Ensure that the 5 excess bits are 0 and that there aren't anymore
    // bytes in the file
    unsigned char readByte = f.get();
    REQUIRE(f.good());
    REQUIRE((readByte & 0x1F) == 0);
    readByte = f.get();
    REQUIRE(f.eof());
    f.close();

    // Clean-up
    remove(filename.c_str());
}

TEST_CASE("can read from BitFileIn", "[bitfile][BitFileIn]")
{
    // Set-up
    std::fstream f;
    BitFileIn filein;
    unsigned char bitOut;
    const std::string filename = "testBitFile2.hex";

    // Make a file with:
    //  3 bits (0b001) to indicate 1 excess bit at the end of file
    // 12 payload bits 0xAD5 (0b1010 1101 0101)
    //  1 excess bit (0) at the end of the file
    f.open(filename, std::fstream::out | std::fstream::binary);
    unsigned int payload = 0xAD5;
    f.put((char)0x35);
    f.put((char)0xAA);
    f.close();

    REQUIRE(!filein.isOpen());
    REQUIRE(!filein.canRead());
    REQUIRE(filein.open(filename));
    REQUIRE(filein.isOpen());
    REQUIRE(filein.canRead());

    SECTION("reading individual bits")
    {
        for (unsigned int mask = 0x800; mask != 0; mask >>= 1)
        {
            INFO("mask: " << std::hex << mask);
            unsigned char verifyBit = (payload & mask) ? 0x01 : 0x00;
            REQUIRE(filein.readBit(bitOut));
            REQUIRE(bitOut == verifyBit);

            if (mask == 0x001)
            {
                REQUIRE(!filein.canRead());
                REQUIRE(!filein.readBit(bitOut));
            }
            else
            {
                REQUIRE(filein.canRead());
            }
        }
    }

    SECTION("reading several bits")
    {
        // Read when there's a sufficient number of bits in the file
        std::vector<bool> bitsOut = filein.readBits(10);
        REQUIRE(bitsOut.size() == 10);
        for (unsigned int mask = 0x800; mask != 0x002; mask >>= 1)
        {
            INFO("mask: " << std::hex << mask);
            bool verifyBit = (payload & mask) ? true : false;
            REQUIRE(verifyBit == bitsOut.front());
            bitsOut.erase(bitsOut.begin());
        }
        REQUIRE(bitsOut.empty());
        REQUIRE(filein.canRead());

        // Try to read more than there is left in the file
        bitsOut = filein.readBits(5);
        REQUIRE(bitsOut.size() == 2);
        REQUIRE(bitsOut.front() == (bool)(payload & 0x002));
        REQUIRE(bitsOut.back() == (bool)(payload & 0x001));
        REQUIRE(!filein.canRead());
    }

    // Clean-up
    filein.close();
    remove(filename.c_str());
}

TEST_CASE("write lots of data to BitFileOut and read it from BitFileIn",
          "[bitfile][BitFileOut][BitFileIn][long]")
{
    const int numDataBytes = 5000;
    const std::string filename = "bigTestBitFile2.hex";

    BitFileOut outfile;
    BitFileIn infile;
    REQUIRE(!outfile.isOpen());
    REQUIRE(!infile.isOpen());

    // Write the data to BitFileOut
    REQUIRE(outfile.open(filename));
    REQUIRE(outfile.isOpen());
    std::vector<unsigned char> databytes;
    databytes.reserve(numDataBytes);
    for (int i = 0; i < numDataBytes; i++)
    {
        unsigned char randByte = rand() % 256;
        REQUIRE(outfile.writeByte(randByte));
        databytes.push_back(randByte);
    }
    REQUIRE(outfile.close());
    REQUIRE(!outfile.isOpen());

    // Read the data from BitFileIn
    REQUIRE(infile.open(filename));
    REQUIRE(infile.isOpen());
    for (int i = 0; i < numDataBytes; i++)
    {
        unsigned char byteRead;
        REQUIRE(infile.readByte(byteRead));
        REQUIRE(byteRead == databytes[i]);
    }
    REQUIRE(!infile.canRead());
    infile.close();
    REQUIRE(!infile.isOpen());

    // Clean-up
    remove(filename.c_str());
}
