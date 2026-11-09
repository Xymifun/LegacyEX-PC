#ifndef GAME_CORE_DATAOUTPUT_HPP
#define GAME_CORE_DATAOUTPUT_HPP

#include <string>

template <class T> class arrayWithLength;
struct PlayerUID;

class DataOutput
{
public:
    virtual ~DataOutput() {}

    virtual void write(unsigned int value) = 0;
    virtual void write(arrayWithLength<unsigned char> data) = 0;
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length) = 0;
    virtual void writeBytes(arrayWithLength<unsigned char> data) = 0;
    virtual void writeByte(unsigned char value) = 0;
    virtual void writeDouble(double value) = 0;
    virtual void writeFloat(float value) = 0;
    virtual void writeInt(int value) = 0;
    virtual void writeLong(long long value) = 0;
    virtual void writeShort(short value) = 0;
    virtual void writeBoolean(bool value) = 0;
    virtual void writeChar(wchar_t value) = 0;
    virtual void writeChars(const std::wstring& text) = 0;
    virtual void writeUTF(const std::wstring& text) = 0;
    virtual void writePlayerUID(PlayerUID uid) = 0;
};

#endif
