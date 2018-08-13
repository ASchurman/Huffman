/* 
 * File:   bitFile.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 */

#ifndef BITFILE_H
#define	BITFILE_H

#include <fstream>
#include <vector>
#include <string>

// TODO: Refactor to use BitVector in BitFileIn and BitFileOut.

/*
Wraps a container in order to store and read bits and bytes.
The advantage of this class is that entire bytes can be retrieved from the
vector at once in the form of a numeric type.
*/
class BitVector {
public:
    // Adds bits to the end of the vector
    void pushBack(bool bit);
    void pushBack(unsigned char byte);

    // Adds bits to the front of the vector
    void pushFront(bool bit);
    void pushFront(unsigned char byte);

    // If the entire vector were packed into bytes, returns the number of bits
    // remaining that wouldn't fit into a byte
    unsigned char remainderBits() { return bitstore.size() % 8; }

    bool canPopBit() { return bitstore.size() >= 1; }
    bool canPopByte() { return bitstore.size() >= 8; }
    unsigned int numBits() { return bitstore.size(); }

    // Gets bits from the vector. Can return with failure if there aren't
    // enough bits in the vector.
    // Returns:
    // (1) bool          - True if successful. 
    // (2) unsigned char - The retrieved bits. This is 0x00 if the retrieval
    //                     failed. In the case of getBit, the least-significant-
    //                     bit is set.
    bool popfrontByte(unsigned char& byteOut);
    bool popfrontBit(unsigned char& bitOut);

private:
    // vector<bool> is typically optimized in order to store each element in
    // a bit rather than in a sizeof(bool).
    std::vector<bool> bitstore;
};

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
    unsigned char buffer = 0x00;

    // Acts as a mask, holding a 1 in the position in the buffer to which we'll
    // write next. When the 1 gets shifted off the end, it's time to flush
    // the buffer.
    // Since we reserve 3 bits at the beginning to indicate the number of
    // unused bits at the end of the final byte, initialize to 0x10.
    unsigned char nextBit = 0x10;
};

/*
Wraps a file stream to read individual bits to file.
As with BitFileOut, the first 3 bits of the file are reserved to indicate how
many unused bits there are in the final byte of the file. This is done because
the number of bits intended to be written may not be divisible by 8.
*/
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
    // bitout - The least-significant-bit is set to the read bit. If the read
    //          is unsuccessful, then this will be 0x00.
    bool readBit(unsigned char& bitOut);

    // Reads a given number of bits from the file.
    // Returns the read bits. The vector<bool> container is optimized to store
    // each bool as a bit rather than as a sizeof(bool).
    // Note that the size of the vector<bool> may be less than numBitsToRead if
    // there's a read error or we've hit EOF.
    std::vector<bool> readBits(int numBitsToRead);

    // Indicates whether one can read bits. If there aren't any more bits to
    // read, returns false.
    bool canRead();

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

