#ifndef GAME_CORE_STRINGFORMAT_HPP
#define GAME_CORE_STRINGFORMAT_HPP

#include <string>

std::wstring formatwstr(const wchar_t* format, ...);
std::string formatstr(const char* format, ...);

#endif
