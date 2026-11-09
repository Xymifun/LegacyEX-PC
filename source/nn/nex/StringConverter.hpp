#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STRINGCONVERTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STRINGCONVERTER_HPP

namespace nn
{
namespace nex
{

class StringConverter
{
public:
    StringConverter(int size);
    ~StringConverter();

    void Encode(const wchar_t* source, bool addTerminator);
    unsigned int GetBufferSize();
    char* GetBuffer();
    void Decode(wchar_t* destination, unsigned short length, bool addTerminator);
    void LimitString(unsigned short length);

private:
    char* m_buffer;
    unsigned int m_bufferSize;
};

}
}

#endif
