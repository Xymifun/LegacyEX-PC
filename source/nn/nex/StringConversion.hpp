#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STRINGCONVERSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STRINGCONVERSION_HPP

namespace nn
{
namespace nex
{

class StringConversion
{
public:
    static int T2Char8(const wchar_t* source, char* destination, unsigned int capacity);
    static int Char8_2T(const char* source, wchar_t* destination, unsigned int capacity);
    static int T2Char16(const wchar_t* source, wchar_t* destination, unsigned int capacity);
    static int Char16_2T(const wchar_t* source, wchar_t* destination, unsigned int capacity);
    static void T2Char8Alloc(const wchar_t* source, char** destination);
    static void T2Char8Release(char* buffer);

    static int Utf8ToT(const char* source, wchar_t* destination, unsigned int capacity);
    static int TToUtf8(const wchar_t* source, char* destination, unsigned int capacity);
    static int Utf8ToWideChar(const char* source, wchar_t* destination, unsigned int capacity);
    static int WideCharToUtf8(const wchar_t* source, char* destination, unsigned int capacity);
    static int GetTToUtf8BufferSize(const wchar_t* source);
    static int GetUtf8ToTBufferSize(const char* source);

    static void Char8ToUtf8Alloc(const char* source, char** destination);
    static void WideCharToUtf8Alloc(const wchar_t* source, char** destination);
    static void Utf8ToWideCharAlloc(const char* source, wchar_t** destination);

    static unsigned char GetCharValue(wchar_t character);
    static unsigned char GetCharValue(char character);
    static void StringToHex(const wchar_t* source, unsigned char* destination, unsigned int count);
    static void StringToHex(const char* source, unsigned char* destination, unsigned int count);
    static int HexToString(const unsigned char* source, wchar_t* destination, unsigned int count);
    static int HexToString(const unsigned char* source, char* destination, unsigned int count);

    static bool ByteArrayToBase64(const unsigned char* bytes, unsigned int size, wchar_t* destination,
                                  unsigned int* length);
    static bool Base64ToByteArray(const wchar_t* source, unsigned int length, unsigned char* destination,
                                  unsigned int* size);
    static bool Encode(const unsigned char* bytes, unsigned int size, wchar_t* destination, unsigned int capacity);
    static bool Decode(const wchar_t* source, unsigned char* destination, unsigned int capacity);
};

}
}

#endif
