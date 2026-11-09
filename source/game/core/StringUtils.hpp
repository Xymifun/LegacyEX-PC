#ifndef GAME_CORE_STRINGUTILS_HPP
#define GAME_CORE_STRINGUTILS_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

bool BothAreSpaces(wchar_t first, wchar_t second);
std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator);
std::vector<std::wstring>& stringSplit(const std::wstring& text, wchar_t separator, std::vector<std::wstring>& result);
std::wstring toLower(const std::wstring& text);
bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second);
std::wstring escapeXML(const std::wstring& text);
std::wstring trimString(const std::wstring& text);
void stripWhitespaceForHtml(std::wstring& text, bool collapse);
std::wstring replaceAll(const std::wstring& text, const std::wstring& search, const std::wstring& replacement);
void makeReplacements(const boost::unordered_map<std::wstring, std::wstring>& replacements, std::wstring& text);
void toUtf8(const std::wstring& text, char** output, unsigned int& length);

#endif
