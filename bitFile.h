/* 
 * File:   bitFile.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 20, 2012
 */

#ifndef BITFILE_H
#define	BITFILE_H

#include <fstream>
#include <vector>
#include <string>

/*
Wraps a file stream to write individual bits to file. If the file already
exists, it is overwritten.
Like std::fstream, BitFileOut must be opened, either with the initializing
constructor or with the open function, before writing.

Because the written bits might not divide evenly into bytes, the first 3 bits
of the file are reserved to indicate how many unused bits there are in the final
byte of the file.
*/
class BitFileOut {
public:
    // Constructs without associating with a file
    BitFileOut() = default;

    virtual ~BitFileOut() noexcept;

    // Initializes by opening the given output file
    BitFileOut(const std::string& filePath);

    BitFileOut(const BitFileOut&) = delete;
    BitFileOut& operator=(const BitFileOut&) = delete;

    BitFileOut(BitFileOut&&) = delete;
    BitFileOut& operator=(BitFileOut&&) = delete;

    // Opens the given file path, returning true if successful
    bool open(const std::string& filePath);

    bool isOpen() { return outfile.is_open(); }
    
    // Flushes the buffer to file and closes the file.
    void close();

    // Writes a single bit. Any non-zero value is read as a 1.
    void writeBit(unsigned char bit);
    
    // Writes a full byte.
    void writeByte(unsigned char bits);

private:
    // The file to which bits are written
    std::fstream outfile;

    // Since only full bytes can be written to file at a time, buffer our bits
    // here until we have a full byte to write.
    unsigned char buffer;

    // Acts as a mask, holding a 1 in the position in the buffer to which we'll
    // write next. When the 1 gets shifted off the end, it's time to flush
    // the buffer.
    // Since we reserve 3 bits at the beginning to indicate the number of
    // unused bits at the end of the final byte, initialize to 0x10.
    unsigned char nextBit = 0x10;
};

#endif	/* BITBUFFER_H */

