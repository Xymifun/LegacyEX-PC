#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STRING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STRING_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

void CopyString(wchar_t** destination, const wchar_t* source);
void CopyString(wchar_t** destination, const char* source);
void CopyString(char** destination, const wchar_t* source);
void CopyString(char** destination, const char* source);

class String : public RootObject
{
public:
    String();
    String(const wchar_t* source);
    String(const char* source);
    String(const String& other);
    virtual ~String();

    String& operator=(const wchar_t* source);
    String& operator=(const char* source);
    String& operator=(const String& other);
    String& operator+=(const String& other);
    bool operator<(const String& other) const;

    String& Format(const wchar_t* format, ...);

    operator const wchar_t*() const { return m_data; }

    operator wchar_t*() { return m_data; }

    unsigned int GetLength() const;
    void Reserve(int size);
    String Truncate(unsigned int length) const;

    void CreateCopy(char** destination) const;
    void CreateCopy(wchar_t** destination) const;
    static void ReleaseCopy(char* copy);
    static void ReleaseCopy(wchar_t* copy);

    void ToUpper();
    void ToLower();

    int FindSubstringCase(const wchar_t* substring, int offset) const;
    int FindSubstringNoCase(const wchar_t* substring) const;
    bool ContainsCase(const String& substring) const;
    bool ContainsNoCase(const String& substring) const;

    unsigned long long ToUInt64() const;

    static bool IsEqual(const wchar_t* left, const wchar_t* right);
    static bool ByteArrayToBase64(const unsigned char* bytes, unsigned int size, wchar_t* destination,
                                  unsigned int capacity);
    static bool Base64ToByteArray(const wchar_t* source, unsigned int length, unsigned char* destination,
                                  unsigned int capacity);
    static bool Base64ToByteArray(const String& source, unsigned char* destination, unsigned int capacity);

    void Trace(unsigned long long flags);
    static void SetDefaultStringEncoding(unsigned int encoding);

private:
    static String _Copy(const wchar_t* first, unsigned int firstLength, const wchar_t* second,
                        unsigned int secondLength);

    friend String operator+(const String& left, const String& right);
    friend String operator+(const String& left, const wchar_t* right);
    friend String operator+(const wchar_t* left, const String& right);

    wchar_t* m_data;
};

String operator+(const String& left, const String& right);
String operator+(const String& left, const wchar_t* right);
String operator+(const wchar_t* left, const String& right);

}
}

#endif
