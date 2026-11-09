#ifndef GAME_CORE_STRINGUTIL_HPP
#define GAME_CORE_STRINGUTIL_HPP

#include <string>

void findAndReplace(std::wstring& value, std::wstring search, std::wstring replacement);
std::wstring escape(std::wstring text);

#endif
