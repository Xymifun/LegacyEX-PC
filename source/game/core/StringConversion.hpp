#ifndef GAME_CORE_STRINGCONVERSION_HPP
#define GAME_CORE_STRINGCONVERSION_HPP

#include <string>

class Block;
class BlockState;
class Direction;

std::wstring char16towstring(wchar_t* text);
void charTowchar_tArray(const char* text, wchar_t* destination, unsigned int capacity);
bool compareStrings(const std::wstring& first, const std::wstring& second);
std::wstring convStringToWstring(wchar_t* text);
std::wstring convStringToWstring(const std::string& text);
std::wstring convUtf8ToWstring(const char* text);
std::string convWstringToString(const std::wstring& text);
wchar_t* stringtowchararray(const std::string& text);

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

template <typename T>
std::wstring _toHexString(T value);

template <typename T>
T _fromHEXString(const std::wstring& value);

#endif
