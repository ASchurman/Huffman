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

TEST_CASE("operating on BitVector", "[bitfile][BitVector]")
{
    bool bit = true;
    unsigned char byte = 0xA1;
    unsigned char popped = 0x00;

    // Operating on empty BitVector
    BitVector v;
    REQUIRE(!v.canPopBit());
    REQUIRE(!v.canPopByte());
    REQUIRE(!v.popfrontBit(popped));
    REQUIRE(popped == 0x00);
    REQUIRE(!v.popfrontByte(popped));
    REQUIRE(popped == 0x00);

    // Push a bit
    v.pushBack(bit);
    REQUIRE(v.numBits() == 1);
    REQUIRE(v.remainderBits() == 1);
    REQUIRE(v.canPopBit());
    REQUIRE(!v.canPopByte());

    // Push a byte
    v.pushBack(byte);
    REQUIRE(v.numBits() == 9);
    REQUIRE(v.remainderBits() == 1);
    REQUIRE(v.canPopBit());
    REQUIRE(v.canPopByte());

    // Pop a bit
    REQUIRE(v.popfrontBit(popped));
    INFO("popped: 0x" << std::hex << +popped);
    REQUIRE(popped == (bit ? 0x01 : 0x00));
    REQUIRE(v.numBits() == 8);
    REQUIRE(v.remainderBits() == 0);
    REQUIRE(v.canPopBit());
    REQUIRE(v.canPopByte());

    // Pop a byte
    REQUIRE(v.popfrontByte(popped));
    INFO("popped: 0x" << std::hex << +popped);
    REQUIRE(popped == byte);
    REQUIRE(v.numBits() == 0);
    REQUIRE(v.remainderBits() == 0);
    REQUIRE(!v.canPopBit());
    REQUIRE(!v.canPopByte());

    // Push to front
    v.pushFront(bit);
    REQUIRE(v.numBits() == 1);
    v.pushFront(bit);
    REQUIRE(v.numBits() == 2);
    v.pushFront(byte);
    REQUIRE(v.numBits() == 10);

    // Pop to verify pushing to front
    REQUIRE(v.popfrontByte(popped));
    INFO("popped: 0x" << std::hex << +popped);
    REQUIRE(popped == byte);
    REQUIRE(v.popfrontBit(popped));
    INFO("popped: 0x" << std::hex << +popped);
    REQUIRE(popped == (bit ? 0x01: 0x00));
    REQUIRE(v.popfrontBit(popped));
    INFO("popped: 0x" << std::hex << +popped);
    REQUIRE(popped == (bit ? 0x01 : 0x00));
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
        INFO("read byte: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +bitsToWrite);
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
        INFO("read byte1: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +bitsToWrite);
        REQUIRE((readByte & 0xE0) >> 5 == 5);

        // Ensure that the correct data bits were written.
        REQUIRE((readByte & 0x1F) == (bitsToWrite & 0xF8) >> 3);
        readByte = f.get();
        INFO("read byte2: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +bitsToWrite);
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
        INFO("read byte1: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +byteToWrite);
        REQUIRE((readByte & 0xE0) >> 5 == 5);

        // Ensure correct data bits were written.
        REQUIRE((readByte & 0x1F) == (byteToWrite & 0xF8) >> 3);
        readByte = f.get();
        INFO("read byte2: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +byteToWrite);
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
        INFO("read byte1: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +bitsToWrite);
        REQUIRE((readByte & 0xE0) >> 5 == 7);

        // Ensure correct data bits were written.
        REQUIRE((readByte & 0x1F) == (bitsToWrite & 0x3E) >> 1);
        readByte = f.get();
        INFO("read byte2: 0x" << std::hex << +readByte << "\n" <<
             "data bits: 0x" << std::hex << +bitsToWrite);
        REQUIRE(readByte == (bitsToWrite & 0x01) << 7);

        // Ensure that there's only 2 bytes in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

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
