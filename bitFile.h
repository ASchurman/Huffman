/* 
 * File:   bitFile.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
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
    
    // Flushes the buffer to file and closes the file. If not called manually,
    // it is called by the destructor.
    // Returns true if the cleanup and close is successful. If it returns false,
    // the file is still closed at the end.
    bool close();

    // Writes a single bit. Any non-zero value is read as a 1.
    // Returns true if successful.
    bool writeBit(unsigned char bit);

    // Writes a number of bits. Each element in vector<bool> is interpreted as
    // a bit: true is 1, false is 0.
    // Returns true if successful.
    bool writeBits(const std::vector<bool>& bits);
    
    // Writes a full byte. Returns true if successful.
    bool writeByte(unsigned char bits);

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

class BitFileIn {
public:
    // Constructs without associating with a file
    BitFileIn() = default;

    virtual ~BitFileIn() noexcept;

    // Initializes by opening the given input file
    BitFileIn(const std::string& filePath);

    BitFileIn(const BitFileIn&) = delete;
    BitFileIn& operator=(const BitFileIn&) = delete;

    BitFileIn(BitFileIn&&) = delete;
    BitFileIn& operator=(BitFileIn&&) = delete;

    // Opens the given file, returning true if successful
    bool open(const std::string& filePath);

    bool isOpen() { return infile.is_open(); }

    // Closes the file. If not called manually, it is called by the destructor.
    void close();

    // Reads a single bit from the file.
    // Returns values:
    // bool - True if the read is successful.
    // unsigned char - The least-significant-bit is set to the read bit. If the
    //                 read is unsuccessful, then this will be 0x00.
    std::pair<bool, unsigned char> readBit();

    // Reads a given number of bits from the file.
    // Returns the read bits. The vector<bool> container is optimized to store
    // each bool as a bit rather than as a sizeof(bool).
    // Note that the size of the vector<bool> may be less than numBitsToRead if
    // there's a read error or we've hit EOF.
    std::vector<bool> readBits(int numBitsToRead);

    // Indicates whether we've read to the end of the file. If a read is
    // unsuccessful and we've NOT hit EOF, then there's a read error.
    bool eof();

private:
    // Reads a new byte from infile into the buffer and resets nextBit.
    // Returns true if successful.
    bool readToBuffer();

    // The file from which to read bits.
    std::ifstream infile;

    // Stores the most recently-read byte from infile. When the caller reads
    // bits, they're read from here.
    unsigned char buffer;

    // Acts as a mask on buffer, with a 1 in the bit position to read next from
    // buffer.
    unsigned char nextBit;

    // The number of unused, remainder bits in the final byte of the file.
    // This value is stored in the first 3 bits of the first byte of the file.
    unsigned char numRemainderBits;
};

#endif	/* BITFILE_H */

