#ifndef GAME_CORE_DATAINPUT_HPP
#define GAME_CORE_DATAINPUT_HPP

#include <string>

template <class T> class arrayWithLength;
struct PlayerUID;

class DataInput
{
public:
    virtual ~DataInput() {}

    virtual int read() = 0;
    virtual int read(arrayWithLength<unsigned char> buffer) = 0;
    virtual int read(arrayWithLength<unsigned char> buffer, unsigned int offset, unsigned int length) = 0;
    virtual bool readBoolean() = 0;
    virtual unsigned char readByte() = 0;
    virtual unsigned char readUnsignedByte() = 0;
    virtual bool readFully(arrayWithLength<unsigned char> buffer) = 0;
    virtual bool readBytes(arrayWithLength<unsigned char> buffer) = 0;
    virtual double readDouble() = 0;
    virtual float readFloat() = 0;
    virtual int readInt() = 0;
    virtual long long readLong() = 0;
    virtual short readShort() = 0;
    virtual wchar_t readChar() = 0;
    virtual std::wstring readUTF() = 0;
    virtual PlayerUID readPlayerUID() = 0;
    virtual long long skipBytes(long long count) = 0;
};

#endif
