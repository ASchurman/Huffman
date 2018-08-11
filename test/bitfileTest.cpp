/*
File: bitfileTest.cpp
Author: Alexander Schurman, alexander.schurman@gmail.com

Provides tests for the classes defined in bitFile.h
*/

#include "catch.hpp"
#include "../bitFile.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

TEST_CASE("can write to BitFileOut", "[bitfile][BitFileOut]")
{
    BitFileOut o;
    const std::string filename = "testBitFile1.hex";

    REQUIRE(!o.isOpen());
    o.open(filename);
    REQUIRE(o.isOpen());

    SECTION("writing individual bits")
    {
        // Write 5 bits, meaning that the entire file will be 8 bits.
        unsigned char randBits = rand() % 0x20;
        for (unsigned char mask = 0x10; mask != 0; mask >>= 1)
        {
            unsigned char bit = (randBits & mask) > 0 ? 1 : 0;
            REQUIRE(o.writeBit(bit));
        }
        REQUIRE(o.close());

        // Ensure that the first 3 bits are 0, indicating that there are no
        // leftover bits at the end of the last (and only) byte.
        std::ifstream f;
        f.open(filename, std::ifstream::in | std::ifstream::binary);
        REQUIRE(f.good());
        unsigned char readByte = f.get();
        REQUIRE((readByte & 0xE0) == 0);

        // Ensure that the correct bits were written
        REQUIRE((readByte & 0x1F) == randBits);

        // Ensure that there's only 1 byte in the file and close it.
        f.get();
        REQUIRE(f.eof());
        f.close();
    }

    // Cleanup
    REQUIRE(!o.isOpen());
    remove(filename.c_str());
}
